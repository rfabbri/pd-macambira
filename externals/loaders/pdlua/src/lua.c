/*

pdlua -- a Lua embedding for Pd
Copyright (C) 2007 Claude Heiland-Allen <claudiusmaximus@goto10.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

/* various C stuff, mainly for reading files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* we use Lua */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* we use Pd */
#include <m_pd.h>

/* defined in pd/src/s_loader.c but not in any header file... */
typedef int (*loader_t)(t_canvas *, char *);
void sys_register_loader(loader_t loader);

/* we need the interpreter state to be global, because we need it in
   our constructor, which doesn't get passed anything useful - blame Pd */
static lua_State *L;

/* state for the file reader */
typedef struct _pdlua_readerdata {
  int fd;
  char buffer[MAXPDSTRING];
} t_pdlua_readerdata;

/* file reader callback */
static const char *pdlua_reader(lua_State *L, void *rr, size_t *size) {
  t_pdlua_readerdata *r = rr;
  ssize_t s;
  s = read(r->fd, r->buffer, MAXPDSTRING-2);
  if (s <= 0) {
    size = 0;
    return NULL;
  } else {
    *size = s;
    return r->buffer;
  }
}

/* declare some stuff in advance */
struct _pdlua_proxyinlet;
struct _pdlua_proxyclock;
/* our object type */
typedef struct _pdlua {
  t_object pd;
  unsigned int inlets;
  struct _pdlua_proxyinlet *in;
  unsigned int outlets;
  t_outlet **out;
  t_canvas *canvas;
} t_pdlua;
/* our dispatchers */
static void pdlua_dispatch(
  t_pdlua *o, unsigned int inlet, t_symbol *s, int argc, t_atom *argv
);
struct _pdlua_proxyreceive;
static void pdlua_receivedispatch(
  struct _pdlua_proxyreceive *r, t_symbol *s, int argc, t_atom *argv
);
static void pdlua_clockdispatch(
  struct _pdlua_proxyclock *clock
);

/* proxy inlet class */

typedef struct _pdlua_proxyinlet {
  t_pd pd;
  struct _pdlua *owner;
  unsigned int id;
} t_pdlua_proxyinlet;

static t_class *pdlua_proxyinlet_class;

static void pdlua_proxyinlet_anything(
  t_pdlua_proxyinlet *p, t_symbol *s, int argc, t_atom *argv
) {
  pdlua_dispatch(p->owner, p->id, s, argc, argv);
}

static void pdlua_proxyinlet_init(
  t_pdlua_proxyinlet *p, struct _pdlua *owner, unsigned int id
) {
  p->pd = pdlua_proxyinlet_class;
  p->owner = owner;
  p->id = id;
}

static void pdlua_proxyinlet_setup() {
  pdlua_proxyinlet_class = class_new(
    gensym("pdlua proxy inlet"),
    0,
    0,
    sizeof(t_pdlua_proxyinlet),
    0,
    0
  );
  class_addanything(
    pdlua_proxyinlet_class, pdlua_proxyinlet_anything
  );
}

/* proxy receive class */
typedef struct _pdlua_proxyreceive {
  t_pd pd;
  struct _pdlua *owner;
  t_symbol *name;
} t_pdlua_proxyreceive;

static t_class *pdlua_proxyreceive_class;

static void pdlua_proxyreceive_anything(
  t_pdlua_proxyreceive *r, t_symbol *s, int argc, t_atom *argv
) {
  pdlua_receivedispatch(r, s, argc, argv);
}

static t_pdlua_proxyreceive *pdlua_proxyreceive_new(
  struct _pdlua *owner, t_symbol *name
) {
  t_pdlua_proxyreceive *r = malloc(sizeof(t_pdlua_proxyreceive));
  r->pd = pdlua_proxyreceive_class;
  r->owner = owner;
  r->name = name;
  pd_bind(&r->pd, r->name);
  return r;
}

static void pdlua_proxyreceive_free(
  t_pdlua_proxyreceive *r
) {
  pd_unbind(&r->pd, r->name);
  r->pd = NULL;
  r->owner = NULL;
  r->name = NULL;
  free(r);
}

static void pdlua_proxyreceive_setup() {
  pdlua_proxyreceive_class = class_new(
    gensym("pdlua proxy receive"),
    0,
    0,
    sizeof(t_pdlua_proxyreceive),
    0,
    0
  );
  class_addanything(
    pdlua_proxyreceive_class, pdlua_proxyreceive_anything
  );
}


/* proxy clock class */

typedef struct _pdlua_proxyclock {
  t_pd pd;
  struct _pdlua *owner;
  t_clock *clock;
} t_pdlua_proxyclock;

static t_class *pdlua_proxyclock_class;

static void pdlua_proxyclock_bang(
  t_pdlua_proxyclock *c
) {
  pdlua_clockdispatch(c);
}

static t_pdlua_proxyclock *pdlua_proxyclock_new(
  struct _pdlua *owner
) {
  t_pdlua_proxyclock *c = malloc(sizeof(t_pdlua_proxyclock));
  c->pd = pdlua_proxyclock_class;
  c->owner = owner;
  c->clock = clock_new(c, (t_method) pdlua_proxyclock_bang);
  return c;
}

static void pdlua_proxyclock_setup() {
  pdlua_proxyclock_class = class_new(
    gensym("pdlua proxy clock"),
    0,
    0,
    sizeof(t_pdlua_proxyclock),
    0,
    0
  );
}

/* dump atoms into a table */
/* called from C */
static void pdlua_pushatomtable(int argc, t_atom *argv) {
  int i;
  lua_newtable(L);
  for (i = 0; i < argc; ++i) {
    lua_pushnumber(L, i+1);
    switch (argv[i].a_type) {
    case A_FLOAT:
      lua_pushnumber(L, argv[i].a_w.w_float);
      break;
    case A_SYMBOL:
      lua_pushstring(L, argv[i].a_w.w_symbol->s_name);
      break;
    case A_POINTER: /* FIXME: check experimentality */
      lua_pushlightuserdata(L, argv[i].a_w.w_gpointer);
      break;
    default:
      error("%s", "lua: zomg weasels!");
      lua_pushnil(L);
      break;
    }
    lua_settable(L, -3);
  }
}

/* constructor dispatcher stub */
/* called from C */
static t_pdlua *pdlua_new(t_symbol *s, int argc, t_atom *argv) {
  lua_getglobal(L, "pd");
  lua_getfield(L, -1, "_constructor");
  lua_pushstring(L, s->s_name);
  pdlua_pushatomtable(argc, argv);
  if (lua_pcall(L, 2, 1, 0)) {
    error(
      "lua: error in constructor for `%s':\n%s",
      s->s_name, lua_tostring(L, -1)
    );
    lua_pop(L, 1);
    return NULL;
  } else {
    t_pdlua *object = NULL;
    if (lua_islightuserdata(L, -1)) {
      object = lua_touserdata(L, -1);
      lua_pop(L, 1);
      return object;
    } else {
      lua_pop(L, 1);
      return NULL;
    }
  }
}

/* destructor dispatcher stub */
/* called from C */
static void pdlua_free(t_pdlua *o) {
  lua_getglobal(L, "pd");
  lua_getfield (L, -1, "_destructor");
  lua_pushlightuserdata(L, o);
  if (lua_pcall(L, 1, 0, 0)) {
    error(
      "lua: error in destructor:\n%s",
      lua_tostring(L, -1)
    );
    lua_pop(L, 1);
  }
  return;
}

/* class registration */
/* called from Lua */
static int pdlua_class_new(lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  t_class *c = class_new(
    gensym((char *) name),
    (t_newmethod) pdlua_new,
    (t_method) pdlua_free,
    sizeof(t_pdlua),
    CLASS_NOINLET,
    A_GIMME,
    0
  );
  lua_pushlightuserdata(L, c);
  return 1;
}

/* object instantiation */
/* called from Lua */
static int pdlua_object_new(lua_State *L) {
  if (lua_islightuserdata(L, 1)) {
    t_class *c = lua_touserdata(L, 1);
    if (c) {
      t_pdlua *o = (t_pdlua *) pd_new(c);
      if (o) {
        o->inlets = 0;
        o->in = NULL;
        o->outlets = 0;
        o->out = NULL;
        o->canvas = canvas_getcurrent();
        lua_pushlightuserdata(L, o);
        return 1;
      }
    }
  }
  return 0;
}

/* inlet creation */
/* called from Lua */
static int pdlua_object_createinlets(lua_State *L) {
  unsigned int i;
  if (lua_islightuserdata(L, 1)) {
    t_pdlua *o = lua_touserdata(L, 1);
    if (o) {
      o->inlets = luaL_checknumber(L, 2);
      o->in = malloc(o->inlets * sizeof(t_pdlua_proxyinlet));
      for (i = 0; i < o->inlets; ++i) {
        pdlua_proxyinlet_init(&o->in[i], o, i);
        inlet_new(&o->pd, &o->in[i].pd, 0, 0);
      }
    }
  }
  return 0;
}

/* outlet creation */
/* called from Lua */
static int pdlua_object_createoutlets(lua_State *L) {
  unsigned int i;
  if (lua_islightuserdata(L, 1)) {
    t_pdlua *o = lua_touserdata(L, 1);
    if (o) {
      o->outlets = luaL_checknumber(L, 2);
      if (o->outlets > 0) {
        o->out = malloc(o->outlets * sizeof(t_outlet *));
        for (i = 0; i < o->outlets; ++i) {
          o->out[i] = outlet_new(&o->pd, 0);
        }
      } else {
        o->out = NULL;
      }
    }
  }
  return 0;
}


/* receive creation */
/* called from Lua */

static int pdlua_receive_new(lua_State *L) {
  if (lua_islightuserdata(L, 1)) {
    t_pdlua *o = lua_touserdata(L, 1);
    if (o) {
      const char *name = luaL_checkstring(L, 2);
      if (name) {
        t_pdlua_proxyreceive *r =  pdlua_proxyreceive_new(o, gensym((char *) name)); /* const cast */
        lua_pushlightuserdata(L, r);
        return 1;
      }
    }
  }
  return 0;
}

static int pdlua_receive_free(lua_State *L) {
  if (lua_islightuserdata(L, 1)) {
    t_pdlua_proxyreceive *r = lua_touserdata(L, 1);
    if (r) {
      pdlua_proxyreceive_free(r);
    }
  }
  return 0;
}


/* clock creation */
/* called from Lua */
static int pdlua_clock_new(lua_State *L) {
  if (lua_islightuserdata(L, 1)) {
    t_pdlua *o = lua_touserdata(L, 1);
    if (o) {
      const char *name = luaL_checkstring(L, 2);
      if (name) {
        t_pdlua_proxyclock *c =  pdlua_proxyclock_new(o);
        lua_pushlightuserdata(L, c);
        return 1;
      }
    }
  }
  return 0;
}

/* clock manipulation */
/* called from Lua */
static int pdlua_clock_delay(lua_State *L) {
  if (lua_islightuserdata(L, 1)) {
    t_pdlua_proxyclock *c = lua_touserdata(L, 1);
    if (c) {
      double delaytime = luaL_checknumber(L, 2);
      clock_delay(c->clock, delaytime);
    }
  }
  return 0;
}

static int pdlua_clock_set(lua_State *L) {
  if (lua_islightuserdata(L, 1)) {
    t_pdlua_proxyclock *c = lua_touserdata(L, 1);
    if (c) {
      double systime = luaL_checknumber(L, 2);
      clock_set(c->clock, systime);
    }
  }
  return 0;
}

static int pdlua_clock_unset(lua_State *L) {
  if (lua_islightuserdata(L, 1)) {
    t_pdlua_proxyclock *c = lua_touserdata(L, 1);
    if (c) {
      clock_unset(c->clock);
    }
  }
  return 0;
}

/* clock destruction */
/* called from Lua */
static int pdlua_clock_free(lua_State *L) {
  if (lua_islightuserdata(L, 1)) {
    t_pdlua_proxyclock *c = lua_touserdata(L, 1);
    if (c) {
      clock_free(c->clock);
      free(c);
    }
  }
  return 0;
}


/* object destruction */
/* called from Lua */
static int pdlua_object_free(lua_State *L) {
  unsigned int i;
  if (lua_islightuserdata(L, 1)) {
    t_pdlua *o = lua_touserdata(L, 1);
    if (o) {
      if (o->in) {
        free(o->in);
      }
      if(o->out) {
        for (i = 0; i < o->outlets; ++i) {
          outlet_free(o->out[i]);
        }
        free(o->out);
        o->out = NULL;
      }
    }
  }
  return 0;
}

/* dispatch methods */
/* called from C */
static void pdlua_dispatch(
  t_pdlua *o, unsigned int inlet, t_symbol *s, int argc, t_atom *argv
) {
  lua_getglobal(L, "pd");
  lua_getfield (L, -1, "_dispatcher");
  lua_pushlightuserdata(L, o);
  lua_pushnumber(L, inlet + 1); /* C has 0.., Lua has 1.. */
  lua_pushstring(L, s->s_name);
  pdlua_pushatomtable(argc, argv);
  if (lua_pcall(L, 4, 0, 0)) {
    pd_error(
      o,
      "lua: error in dispatcher:\n%s",
      lua_tostring(L, -1)
    );
    lua_pop(L, 1);
  }
  return;  
}


/* dispatch receives */
/* called from C */
static void pdlua_receivedispatch(
  t_pdlua_proxyreceive *r, t_symbol *s, int argc, t_atom *argv
) {
  lua_getglobal(L, "pd");
  lua_getfield (L, -1, "_receivedispatch");
  lua_pushlightuserdata(L, r);
  lua_pushstring(L, s->s_name);
  pdlua_pushatomtable(argc, argv);
  if (lua_pcall(L, 3, 0, 0)) {
    pd_error(
      r->owner,
      "lua: error in receive dispatcher:\n%s",
      lua_tostring(L, -1)
    );
    lua_pop(L, 1);
  }
  return;  
}


/* dispatch clocks */
/* called from C */
static void pdlua_clockdispatch(
  t_pdlua_proxyclock *clock
) {
  lua_getglobal(L, "pd");
  lua_getfield (L, -1, "_clockdispatch");
  lua_pushlightuserdata(L, clock);
  if (lua_pcall(L, 1, 0, 0)) {
    pd_error(
      clock->owner,
      "lua: error in clock dispatcher:\n%s",
      lua_tostring(L, -1)
    );
    lua_pop(L, 1);
  }
  return;  
}

/* grab a Lua table into an array of atoms */
static t_atom *pdlua_popatomtable(lua_State *L, int *count, t_pdlua *o) {
  int i;
  int ok;
  t_float f;
  const char *s;
  void *p;
  size_t sl;
  t_atom *atoms;
  atoms = NULL;
  ok = 1;
  if (lua_istable(L, -1)) {
    *count = lua_objlen(L, -1);
    if (*count > 0) {
      atoms = malloc(*count * sizeof(t_atom));
    }
    i = 0;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
      if (i == *count) {
        pd_error(o, "%s", "lua: error: too many table elements");
        ok = 0;
        break;
      }
      switch (lua_type(L, -1)) {
      case (LUA_TNUMBER):
        f = lua_tonumber(L, -1);
        SETFLOAT(&atoms[i], f);
        break;
      case (LUA_TSTRING):
        s = lua_tolstring(L, -1, &sl);
        if (s) {
          if (strlen(s) != sl) {
            pd_error(o, "%s", "lua: warning: symbol munged (contains \\0 in body)");
          }
          SETSYMBOL(&atoms[i], gensym((char *) s));
        } else {
          pd_error(o, "%s", "lua: error: null string in table");
          ok = 0;
        }
        break;
      case (LUA_TLIGHTUSERDATA): /* FIXME: check experimentality */
        p = lua_touserdata(L, -1);
        SETPOINTER(&atoms[i], p);
        break;
      default:
        pd_error(o, "s", "lua: error: table element must be number or string or pointer");
        ok = 0;
        break;
      }
      lua_pop(L, 1);
      ++i;
    }
    if (i != *count) {
      pd_error(o, "%s", "lua: error: too few table elements");
      ok = 0;
    }
  } else {
    pd_error(o, "%s", "lua: error: not a table");
    ok = 0;
  }
  lua_pop(L, 1);
  if (ok) {
    return atoms;
  } else {
    if (atoms) {
      free(atoms);
    }
    return NULL;
  }
}

/* output to outlets */
/* called from Lua */
static int pdlua_outlet(lua_State *L) {
  t_pdlua *o;
  int out;
  size_t sl;
  const char *s;
  t_symbol *sym;
  int count;
  t_atom *atoms;
  if (lua_islightuserdata(L, 1)) {
    o = lua_touserdata(L, 1);
    if (o) {
      if (lua_isnumber(L, 2)) {
        out = lua_tonumber(L, 2) - 1; /* C has 0.., Lua has 1.. */
      } else {
        pd_error(o, "%s", "lua: error: outlet must be a number");
        return 0;
      }
      if (0 <= out && out < o->outlets) {
        if (lua_isstring(L, 3)) {
          s = lua_tolstring(L, 3, &sl);
          sym = gensym((char *) s); /* const cast */
          if (s) {
            if (strlen(s) != sl) {
              pd_error(o, "%s", "lua: warning: symbol munged (contains \\0 in body)");
            }
            lua_pushvalue(L, 4);
            atoms = pdlua_popatomtable(L, &count, o);
            if (count == 0 || atoms) {
              outlet_anything(o->out[out], sym, count, atoms);
            } else {
              pd_error(o, "%s", "lua: error: no atoms??");
            }
            if (atoms) {
              free(atoms);
              return 0;
            }
          } else {
            pd_error(o, "%s", "lua: error: null selector");
          }
        } else {
          pd_error(o, "%s", "lua: error: selector must be a string");
        }
      } else {
        pd_error(o, "%s", "lua: error: outlet out of range");
      }
    } else {
      error("lua: error: no object to outlet from");
    }
  } else {
    error("lua: error: bad arguments to outlet");
  }
  return 0;
}
/* send to receive-names */
/* called from Lua */
static int pdlua_send(lua_State *L) {
  size_t receivenamel;
  const char *receivename;
  t_symbol *receivesym;
  size_t selnamel;
  const char *selname;
  t_symbol *selsym;
  int count;
  t_atom *atoms;
  if (lua_isstring(L, 1)) {
    receivename = lua_tolstring(L, 1, &receivenamel);
    receivesym = gensym((char *) receivename); /* const cast */
    if (receivesym) {
      if (strlen(receivename) != receivenamel) {
        error("%s", "lua: warning: symbol munged (contains \\0 in body)");
      }
      if (lua_isstring(L, 2)) {
        selname = lua_tolstring(L, 2, &selnamel);
        selsym = gensym((char *) selname); /* const cast */
        if (selsym) {
          if (strlen(selname) != selnamel) {
            error("%s", "lua: warning: symbol munged (contains \\0 in body)");
          }
          lua_pushvalue(L, 3);
          atoms = pdlua_popatomtable(L, &count, NULL);
          if (count == 0 || atoms) {
            if (receivesym->s_thing) {
              typedmess(receivesym->s_thing, selsym, count, atoms);
            }
          } else {
            error("%s", "lua: error: no atoms??");
          }
          if (atoms) {
            free(atoms);
            return 0;
          }
        } else {
          error("%s", "lua: error: null selector");
        }
      } else {
        error("%s", "lua: error: selector must be a string");
      }
    } else {
      error("%s", "lua: error: null receive name");
    }
  } else {
    error("%s", "lua: error: receive name must be string");
  }
  return 0;
}

/* post to the console */
/* called from Lua */
static int pdlua_post(lua_State *L) {
  const char *str = luaL_checkstring(L, 1);
  post("%s", str);
  return 0;
}

/* report errors gracefully */
/* called from Lua */
static int pdlua_error(lua_State *L) {
  t_pdlua *o;
  const char *s;
  if (lua_islightuserdata(L, 1)) {
    o = lua_touserdata(L, 1);
    if (o) {
      s = luaL_checkstring(L, 2);
      if (s) {
        pd_error(o, "%s", s);
      } else {
        pd_error(o, "%s", "lua: error: null string in error function");
      }
    } else {
      error("lua: error: null object in error function");
    }
  } else {
    error("lua: error: bad arguments to error function");
  }
  return 0;
}

/* run a Lua script using Pd's path */
/* called from Lua */
static int pdlua_dofile(lua_State *L) {
  char buf[MAXPDSTRING];
  char *ptr;
  t_pdlua_readerdata reader;
  int fd;
  int n;
  const char *filename;
  t_pdlua * o;
  n = lua_gettop(L);
  if (lua_islightuserdata(L, 1)) {
    o = lua_touserdata(L, 1);
    if (o) {
      filename = luaL_optstring(L, 2, NULL);
      fd = canvas_open(o->canvas, filename, "", buf, &ptr, MAXPDSTRING, 1);
      if (fd >= 0) {
        reader.fd = fd;
        if (lua_load(L, pdlua_reader, &reader, filename)) {
          close(fd);
          lua_error(L);
        } else {
          if (lua_pcall(L, 0, LUA_MULTRET, 0)) {
            pd_error(
              o,
              "lua: error running `%s':\n%s",
              filename,
              lua_tostring(L, -1)
            );
            lua_pop(L, 1);
            close(fd);
          } else {
            /* succeeded */
            close(fd);
          }
        }
      } else {
        pd_error(o, "lua: error loading `%s': canvas_open() failed", filename);
      }
    } else {
      error("lua: error in object:dofile() - object is null");
    }
  } else {
    error("lua: error in object:dofile() - object is wrong type");
  }
  return lua_gettop(L) - n;
}

/* initialize pd API for Lua */
/* called from C */
static void pdlua_init(lua_State *L) {
  lua_newtable(L);
  lua_setglobal(L, "pd");
  lua_getglobal(L, "pd");
  lua_pushstring(L, "_register");
  lua_pushcfunction(L, pdlua_class_new);
  lua_settable(L, -3);
  lua_pushstring(L, "_create");
  lua_pushcfunction(L, pdlua_object_new);
  lua_settable(L, -3);
  lua_pushstring(L, "_createinlets");
  lua_pushcfunction(L, pdlua_object_createinlets);
  lua_settable(L, -3);
  lua_pushstring(L, "_createoutlets");
  lua_pushcfunction(L, pdlua_object_createoutlets);
  lua_settable(L, -3);
  lua_pushstring(L, "_destroy");
  lua_pushcfunction(L, pdlua_object_free);
  lua_settable(L, -3);
  lua_pushstring(L, "_outlet");
  lua_pushcfunction(L, pdlua_outlet);
  lua_settable(L, -3);
  lua_pushstring(L, "_createreceive");
  lua_pushcfunction(L, pdlua_receive_new);
  lua_settable(L, -3);
  lua_pushstring(L, "_receivefree");
  lua_pushcfunction(L, pdlua_receive_free);
  lua_settable(L, -3);
  lua_pushstring(L, "_createclock");
  lua_pushcfunction(L, pdlua_clock_new);
  lua_settable(L, -3);
  lua_pushstring(L, "_clockfree");
  lua_pushcfunction(L, pdlua_clock_free);
  lua_settable(L, -3);
  lua_pushstring(L, "_clockset");
  lua_pushcfunction(L, pdlua_clock_set);
  lua_settable(L, -3);
  lua_pushstring(L, "_clockunset");
  lua_pushcfunction(L, pdlua_clock_unset);
  lua_settable(L, -3);
  lua_pushstring(L, "_clockdelay");
  lua_pushcfunction(L, pdlua_clock_delay);
  lua_settable(L, -3);
  lua_pushstring(L, "_dofile");
  lua_pushcfunction(L, pdlua_dofile);
  lua_settable(L, -3);
  lua_pushstring(L, "send");
  lua_pushcfunction(L, pdlua_send);
  lua_settable(L, -3);
  lua_pushstring(L, "post");
  lua_pushcfunction(L, pdlua_post);
  lua_settable(L, -3);
  lua_pushstring(L, "_error");
  lua_pushcfunction(L, pdlua_error);
  lua_settable(L, -3);
  lua_pop(L, 1);
}

/* Pd loader hook for loading and executing Lua scripts */
static int pdlua_loader(t_canvas *canvas, char *name) {
  char buf[MAXPDSTRING];
  char *ptr;
  int fd;
  t_pdlua_readerdata reader;
  fd = canvas_open(canvas, name, ".lua", buf, &ptr, MAXPDSTRING, 1);
  if (fd >= 0) {
    reader.fd = fd;
    if (lua_load(L, pdlua_reader, &reader, name) ||
        lua_pcall(L, 0, 0, 0)) {
      error(
        "lua: error loading `%s':\n%s",
        name,
        lua_tostring(L, -1)
      );
      lua_pop(L, 1);
      close(fd);
      return 0;
    }
    close(fd);
    return 1;
  } else {
    return 0;
  }
}

/* start the Lua runtime and register our loader hook */
extern void lua_setup(void) {
  char buf[MAXPDSTRING];
  char *ptr;
  t_pdlua_readerdata reader;
  int fd;
  post("lua 0.3 (GPL) 2007 Claude Heiland-Allen <claudiusmaximus@goto10.org>");
  pdlua_proxyinlet_setup();
  pdlua_proxyreceive_setup();
  pdlua_proxyclock_setup();
  L = lua_open();
  luaL_openlibs(L);
  pdlua_init(L);
  /* "pd.lua" is the Lua part of pdlua, want to keep the C part minimal */
  fd = canvas_open(0, "pd", ".lua", buf, &ptr, MAXPDSTRING, 1);
  if (fd >= 0) {
    reader.fd = fd;
    if (lua_load(L, pdlua_reader, &reader, "pd.lua") ||
        lua_pcall(L, 0, 0, 0)) {
      error(
        "lua: error loading `pd.lua':\n%s",
        lua_tostring(L, -1)
      );
      error("lua: loader will not be registered!");
      lua_pop(L, 1);
      close(fd);
    } else {
      close(fd);
      sys_register_loader(pdlua_loader);
    }
  } else {
    error("lua: error loading `pd.lua': canvas_open() failed");
    error("lua: loader will not be registered!");
  }
}
