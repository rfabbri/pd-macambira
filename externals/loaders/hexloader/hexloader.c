/* Copyright (c) 2007 IOhannes m zmölnig @ IEM
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," that comes with Pd.  
 */

/*
 * this code adds an external "loader" to Miller S. Puckette's "pure data",
 * which allows the loading of libraries/externals with special characters
 * in the classname
 *
 * the infrastructure of this file is based on hcsteiner's "libdir" loader
 */


#ifdef __WIN32__
# define MSW
#endif

#include "m_pd.h"
#include "s_stuff.h"
#include "g_canvas.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef DL_OPEN
# include <dlfcn.h>
#endif
#ifdef UNISTD
# include <stdlib.h>
# include <unistd.h>
#endif
#ifdef __WIN32__
# include <io.h>
# include <windows.h>
#endif
#ifdef __APPLE__
# include <mach-o/dyld.h> 
#endif


typedef void (*t_hexloader_setup)(void);

/* definitions taken from s_loader.c  */
typedef int (*loader_t)(t_canvas *canvas, char *classname);
void sys_register_loader(loader_t loader);
void class_set_extern_dir(t_symbol *s);

/* ==================================================== */

typedef struct _hexloader
{
  t_object x_obj;
} t_hexloader;
static t_class *hexloader_class;

static char *version = "$Revision: 1.3 $";


static char*hex_dllextent[] = {
#ifdef __FreeBSD__
  ".b_i386", ".pd_freebsd",
#endif
#ifdef __linux__
# ifdef __x86_64__
  ".l_ia64",
# else
  ".l_i386",
# endif
  ".pd_linux",
#endif /* linux */
#ifdef __APPLE__
# ifndef MACOSX3
  ".d_fat",
# else
  ".d_ppc",
# endif
  ".pd_darwin", 
#endif
#ifdef __WIN32__
  ".m_i386", ".dll",
#endif
  0}; /* zero-terminated list of extensions */



static  char *patch_extent[]={
  ".pd", 
  ".pat", 
  0};


/**
 * object-names containing non-alphanumerics like [||~]
 * can (sometimes) be not represented on filesystems (e.g.
 * "|" is a forbidden character) and more often they
 * cannot be used to construct a valid setup-function
 * ("||~_setup" or "||_tilde_setup" are really bad).
 * the way the "~" is handled, is non-generic and thus
 * sub-optimal.
 *
 * as a solution me and hcs proposed an encoding into
 * alphanumeric-values, using a hexadecimal representation
 * of all characters but [0-9A-Za-z_] (e.g. "+" is ascii
 * 43 and is thus represented by "0x2b" (hex-value all
 * lowercase and prepended with "0x")
 *
 * e.g. if we have a new class "mtx_||", pd first attempts
 * to find a file called "mtx_||.dll". if it succeeds, it
 * will try to call the "mtx_||_setup()" function.
 * if that fails we suggest to try and call a function
 * "setup_mtx_0x7c0x7c()" (the keyword setup is now at the
 * beginning of the function-name, in order to prevent the
 * names starting with numbers and in order to distinguish
 * between the normal setup-methods).
 * if no "mtx_||.dll" can be found, pd should then search
 * for a file "mtx_0x7c0x7c.dll" (guaranteed to be
 * representable on any filesystem); search this file for
 * the 2 setup-functions.
 * if all fails try to find "mtx_||.pd" and then
 * "mtx_0x7c0x7c.pd"
 */

/*
 * ideally this loader could somehow call the object-instantiator recursively
 * but with changed classnames;
 * this would allow us to use the hexloader stuff for all kinds of other loaders
 * including abstractions
 */

/* -------------------- utilities --------------------- */


/* --- namelist_t: a linked list of names --- */

/* linked list of loaders */
typedef struct namelist_ {
    char* name;
    struct namelist_ *next;
} namelist_t;

static void namelist_print(char*prefix, namelist_t*names) {
    for(; names; names=names->next) {
      if(prefix)startpost("%s:: ", prefix);
      post("%s",names->name);
    }
}

static namelist_t*namelist_add(namelist_t*names, char*name) {
  namelist_t*dummy=names;
  namelist_t*last=0;

  //  namelist_print("adder contained:", names);

  if(name==0)return names;

  if(!dummy) {
    dummy=(namelist_t*)getbytes(sizeof(namelist_t));
    dummy->next=0;
    dummy->name=name;
    return dummy;
  }

  for(; dummy; dummy=dummy->next) {
    //    post("checking '%s' vs '%s'", name, dummy->name);
    if (!strncmp(name, dummy->name, MAXPDSTRING)) {
      // we already have this entry!
      //      post("found name %s=%s", name, dummy->name);
      return names;
    }
    last=dummy;
  }
  dummy=last;

  dummy->next=(namelist_t*)getbytes(sizeof(namelist_t));
  dummy=dummy->next;
  dummy->next=0;
  dummy->name=name;

  //  namelist_print("adder contains::", names);

  return names;
}

static namelist_t*namelist_addlist(namelist_t*names, namelist_t*nl) {
  namelist_t*dummy=0;
  if(nl==0)return names;
  if(!names)return nl;

  /* try to add each entry in nl */
  for(dummy=nl; dummy->next; dummy=dummy->next) {
    names=namelist_add(names, dummy->name);
  }

  return names;
}

static void namelist_clear(namelist_t*names) {
  namelist_t*dummy=0;

  while(names) {
    dummy=names->next;
    names->next=0;
    names->name=0; /* we dont care since the names are allocated in the symboltable anyhow */
    freebytes(names, sizeof(namelist_t));
    names=dummy;
  }
}

/* --- filelist_t: a linked list of filenames, each associated with a list of setupfunction names --- */

typedef struct filelist_ {
  char *name;
  namelist_t*setupfun;
  struct filelist_ *next;
} filelist_t;

static void filelist_print(char*prefix, filelist_t*files) {
  for(; files; files=files->next) {
    if(prefix)startpost("%s: ", prefix);
    post("%s",files->name);
    namelist_print("\t", files->setupfun);
  }
}

static filelist_t*filelist_add(filelist_t*files, char*name, namelist_t*setupfun) {
  filelist_t *last=0, *dummy=files;
  if(name==0)return files;

  if(!dummy) {
    dummy=(filelist_t*)getbytes(sizeof(filelist_t));
    dummy->next=0;
    dummy->name=name;
    dummy->setupfun=namelist_addlist(0, setupfun);
    return dummy;
  }

  for(; dummy; dummy=dummy->next) {
    if (!strncmp(name, dummy->name, MAXPDSTRING)) {
      // we already have this entry!
      /* add additional setup-functions to this name */
      dummy->setupfun=namelist_addlist(dummy->setupfun, setupfun);
      return files;
    }
    last=dummy;
  }
  dummy=last;

  /* this is a new entry, add it to the list */

  dummy->next=(filelist_t*)getbytes(sizeof(filelist_t));
  dummy=dummy->next;
  dummy->next=0;
  dummy->name=name;
  dummy->setupfun=namelist_addlist(0, setupfun);

  return files;
}

static filelist_t*filelist_addlist(filelist_t*files, filelist_t*nl) {
  filelist_t*dummy=0;
  if(nl==0)return files;
  if(!files)return nl;

  /* try to add each entry in nl */
  for(dummy=nl; dummy->next; dummy=dummy->next) {
    files=filelist_add(files, dummy->name, dummy->setupfun);
  }

  return files;
}

static void filelist_clear(filelist_t*files) {
  filelist_t*dummy=0;

  while(files) {
    dummy=files->next;
    namelist_clear(files->setupfun);
    files->setupfun=0;
    files->name=0; /* we dont care since the files are allocated in the symboltable anyhow */
    files->next=0;
    freebytes(files, sizeof(filelist_t));
    files=dummy;
  }
}



/* ---------------- normalize names ------------------- */


/**
 * replace everything but [a-zA-Z0-9_] by "0x%x" 
 * @return the normalized version of org
 */
static char*hexloader_normalize(char*org, int skipslash)
{
  char*orgname=org;
  char altname[MAXPDSTRING];
  t_symbol*s=0;

  int count=0;
  int i=0;

  for(i=0; i<MAXPDSTRING; i++)
    altname[i]=0;

  i=0;
  while(*orgname && i<MAXPDSTRING)
    {
      char c=*orgname;
      if((c>='0' && c<='9')|| /* [0-9] */
         (c>='A' && c<='Z')|| /* [A-Z] */
         (c>='a' && c<='z')||/* [a-z] */
         (c=='_') ||        /* [_] */
         (skipslash && c=='/')
         )
        {
          altname[i]=c;
          i++;
        }
      else /* a "bad" character */
        {
          sprintf(altname+i, "0x%02x", c);
          i+=4;
          count++;
        }
      orgname++;
    }

  s=gensym(altname);
  //  post("normalize=%s", s->s_name);
  return s->s_name;
}

/**
 * replace only / \ : * ? " < > | by 0x%x since these are forbidden on some filesystems
 * @return the normalized version of org
 */
static char*hexloader_fsnormalize(char*org)
{
  char*orgname=org;
  char altname[MAXPDSTRING];
  t_symbol*s=0;

  char forbiddenchars[]={ 
    //    '/', '\\', ':', '*', '?', '"', '<', '>', '|',
    '\\', ':', '*', '?', '"', '<', '>', '|',
    0};

  int count=0;
  int i=0;

  for(i=0; i<MAXPDSTRING; i++)
    altname[i]=0;

  i=0;
  while(*orgname && i<MAXPDSTRING)
    {
      char c=*orgname;
      char*forbidden=forbiddenchars;
      int found=0;

      while(*forbidden) {
        if(c==*forbidden) {
          sprintf(altname+i, "0x%02x", c);
          i+=4;
          count++;
          found=1;
          break;
        }
        forbidden++;
      }
      if(!found)
        {
          altname[i]=c;
          i++;
        }
      orgname++;      
    }

  s=gensym(altname);
  //  post("fsnormalize=%s", s->s_name);
  return s->s_name;
}


static filelist_t*hexloader_deslashify(filelist_t*names, char*prefix, char*postfix) {
  filelist_t*result=names;
  int i=0;
  char*cp=0;
  char buf[MAXPDSTRING];
  t_symbol*s;

  snprintf(buf, MAXPDSTRING, "%s%s", prefix, postfix);
  s=gensym(buf);

  result=filelist_add(result, s->s_name, 0);

  for(cp=postfix; *cp; cp++) {
    if(*cp=='/') {
      char *postfix2=postfix+i+1;
      char buf2[MAXPDSTRING];

      snprintf(buf2, MAXPDSTRING, "%s", prefix);
      strncat(buf2, postfix, i);
      strcat(buf2, "/");
      result=hexloader_deslashify(result, buf2, postfix2);

      snprintf(buf2, MAXPDSTRING, "%s", prefix);
      strncat(buf2, postfix, i);
      strcat(buf2, "0x2f");
      result=hexloader_deslashify(result, buf2, postfix2);

    }
    i++;
  }

  return result;
}


static namelist_t*hexloader_fsnormalize_list(namelist_t*names, char*org) {
  char*simple=hexloader_fsnormalize(org);
  names=namelist_add(names, org);
  names=namelist_add(names, simple);

  return names;
}



/* ---------------- core code ------------------- */

static namelist_t*hexloader_getaltsetups(namelist_t*names, char*name) {
  char sbuf[MAXPDSTRING];
  t_symbol*s;
  snprintf(sbuf, MAXPDSTRING, "%s_setup", name);
  s=gensym(sbuf);
  names=namelist_add(names, s->s_name);
  snprintf(sbuf, MAXPDSTRING, "setup_%s", name);
  s=gensym(sbuf);
  names=namelist_add(names, s->s_name);
  return names;
}
static namelist_t*hexloader_getaltsetup(namelist_t*names, char*name) {
  namelist_t*result=names;
  char*strippedname=name;
  char*cp=name;

  while(*cp++) {
    if(*cp=='/'){
      strippedname=cp+1;
    }
  }
 
  result=hexloader_getaltsetups(result, strippedname);
  result=hexloader_getaltsetups(result, hexloader_normalize(strippedname, 0));
  //  post("added to %s\n", name);
  return result;
}

static filelist_t*hexloader_fillaltsetupfuns(filelist_t*files) {
  filelist_t*f;
  for(f=files; f; f=f->next) {
    f->setupfun=namelist_addlist(f->setupfun, hexloader_getaltsetup(f->setupfun, f->name));
  }
  return files;
}

/**
 * @return a 0-terminated array of all filenames we consider to be alternative names and associated setup-functions
 */

static filelist_t*hexloader_getalternatives(char*org) {
  filelist_t*files=0;

  files=filelist_add(files, org, 0);
  files=filelist_add(files, hexloader_fsnormalize(org), 0);
  files=filelist_add(files, hexloader_normalize(org, 1),0);
  files=filelist_add(files, hexloader_normalize(org, 0),0);
  files=hexloader_deslashify(files, "", org);

  hexloader_fillaltsetupfuns(files);
  //  filelist_print("ALTER: ", files);

  return files;
}

static int hexloader_doload(char*filename, char*setupfun) {
#ifdef __WIN32__
    HINSTANCE ntdll;
#endif
    t_hexloader_setup makeout=0;

#ifdef DL_OPEN
    void *dlobj = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
    if (!dlobj)
    {
      verbose(2, "%s: %s", filename, dlerror());
      class_set_extern_dir(&s_);
      return (0);
    }
    makeout = (t_hexloader_setup)dlsym(dlobj,  setupfun);
#endif
#ifdef __WIN32__
    sys_bashfilename(filename, filename);
    ntdll = LoadLibrary(filename);
    if (!ntdll)
    {
      verbose(2, "%s: couldn't load", filename);
      class_set_extern_dir(&s_);
      return (0);
    }
    makeout = (t_hexloader_setup)GetProcAddress(ntdll, setupfun);  
#endif

    if (!makeout)
    {
      verbose(2, "hexload object: Symbol \"%s\" not found", setupfun);
      class_set_extern_dir(&s_);
      return 0;
    }
    (*makeout)();
    return (1);
}

/**
 * open a file (given via pathname+filename) as dll and call various
 * setupfunctions (as can be calculated from the altnames array
 * @param pathname the path of the file
 * @param filename the name (without path) of the file
 * @param altnames a zero-terminated array of possible non-generic parts of the setup-function
 * @return 1 on success, 0 otherwise
 */
static int hexloader_loadfile(char*pathname, char*filename, namelist_t*altnames) 
{
  char setupfun[MAXPDSTRING];
  char fullfile[MAXPDSTRING];
  namelist_t*altname=altnames;

  sprintf(fullfile, "%s/%s", pathname, filename);

  while(altname) {
    if(hexloader_doload(fullfile, altname->name))
      return 1;

    altname=altname->next;
  }

  return 0;
}


static void*hexloader_new(t_symbol *s, int argc, t_atom *argv);
/**
 * try to open a file (given via pathname+filename) as a patcher
 * TODO: make this work....
 * @param pathname the path of the file
 * @param filename the name (without path) of the file
 * @param altclassname the alternative classname we currently use...
 * @return 1 on success, 0 otherwise
 */

/* this only gets called if we have already found an abstraction-file */
static int hexloader_loadpatch(char*pathname, char*filename, char*altclassname, char*realclassname)
{
  char fullfile[MAXPDSTRING];
  sprintf(fullfile, "%s/%s", pathname, filename);

  post("BUG: hexloader not loading patch: %s (not yet implemented)", fullfile);
  return 0;
}
/**
 * the actual loader:
 *  example-nomenclature:
 *   "class": the original class-name (e.g. containing weird characters)
 *   "CLASS": the normalized class-name (with all weirdness replaced by hex-representations
 *   "ext"  : the external-extension (e.g. "dll" or "pd_linux" or...)
 *  example:
 *   trying to create an object [class] (and pd fails for whatever reasons, and thus callsus)
 *   - search for a file "class.ext" in our search-path
 *    + if found
 *     try to call "class_setup()" function 
 *     if fails, try to call "setup_CLASS()" function
 *    (if fails, try to call "CLASS_setup()" function)
 *    - "class.ext" file not found
 *   - search for a file "CLASS.ext" in our search-path
 *    + if found
 *     try to call "class_setup()" function 
 *     if fails, try to call "setup_CLASS()" function
 *    (if fails, try to call "CLASS_setup()" function)
 *   - if everything fails, return...
 *
 * @param canvas the context of the object to be created
 * @param classname the name of the object (external, library) to be created
 * @return 1 on success, 0 on failure
 */
static int hexloader_doloader(t_canvas *canvas, filelist_t*altnames0, char*classname)
{
  int fd = -1;
  char dirbuf[MAXPDSTRING];
  char*nameptr;
  filelist_t*altnames=altnames0;
  
  /* try binaries */
  while(altnames) {
    char*altname=altnames->name;
    int dll_index=0;
    char*dllextent=hex_dllextent[dll_index];
    while(dllextent!=0) {
      if ((fd = open_via_path(".", altname, dllextent, dirbuf, &nameptr, MAXPDSTRING, 0)) >= 0) {
        close (fd);

        if(hexloader_loadfile(dirbuf, nameptr, altnames->setupfun)) {
          return 1;
        }
      }
      dll_index++;
      dllextent=hex_dllextent[dll_index];
    }

    altnames=altnames->next;
  }

  /* try patches */
  altnames=altnames0;
  while(altnames) {
    char*altname=altnames->name;
    int extindex=0;
    char*extent=patch_extent[extindex];
    while(extent!=0) {
      if ((fd = open_via_path(".", altname, extent, dirbuf, &nameptr, MAXPDSTRING, 0)) >= 0) {
        close (fd);
        if(hexloader_loadpatch(dirbuf, nameptr, altname, classname)) {
          return 1;
        }
      }

      extindex++;
      extent=patch_extent[extindex];
    }
    altnames=altnames->next;
  }
  return 0;
}

/**
 * the loader
 *
 * @param canvas the context of the object to be created
 * @param classname the name of the object (external, library) to be created
 * @return 1 on success, 0 on failure
 */
static int hexloader_loader(t_canvas *canvas, char *classname)
{
  filelist_t*altnames=0;
  int result=0;

  static int already_loading=0;
  if(already_loading)return 0;
  already_loading=1;

  /* get alternatives */
  altnames=hexloader_getalternatives(classname);

  /* do the loading */
  result=hexloader_doloader(canvas, altnames, classname);

  /* clean up */
  filelist_clear(altnames); 

  already_loading=0;
  return result;
}

static void*hexloader_new(t_symbol *s, int argc, t_atom *argv)
{
  if(s==gensym("hexloader")) {
    t_hexloader*x = (t_hexloader*)pd_new(hexloader_class);
    return (x);
  }

  return 0;
}

void hexloader_setup(void)
{
  /* relies on t.grill's loader functionality, fully added in 0.40 */
  post("hex loader %s",version);  
  post("\twritten by IOhannes m zmölnig, IEM <zmoelnig@iem.at>");
  post("\tcompiled on "__DATE__" at "__TIME__ " ");
  post("\tcompiled against Pd version %d.%d.%d.%s", PD_MAJOR_VERSION, PD_MINOR_VERSION, PD_BUGFIX_VERSION, PD_TEST_VERSION);

#if (PD_MINOR_VERSION >= 40)
  sys_register_loader(hexloader_loader);
#else
  error("to function, this needs to be compiled against Pd 0.40 or higher,\n");
  error("\tor a version that has sys_register_loader()");
#endif

  hexloader_class = class_new(gensym("hexloader"), (t_newmethod)hexloader_new, 0, sizeof(t_hexloader), CLASS_NOINLET, A_GIMME, 0);
}
