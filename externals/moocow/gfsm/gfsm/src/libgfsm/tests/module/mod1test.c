#include <stdio.h>
#include <glib.h>
#include <gmodule.h>

typedef void (*fooFunc) (void);

int main(int argc, char **argv) {
  int i;
  const gchar *prog = argv[0];
  //const gchar *moddir = g_getenv("PWD");  //-- get module directory (hack)
  const gchar *moddir = ".";
  gchar *modpath;
  GModule *mod;
  fooFunc foofunc;

  g_assert(g_module_supported());

  for (i=1; i < argc; i++) {
    //-- build full module pathname
    modpath = g_module_build_path(moddir,argv[i]);
    printf("%s: argv[%d]='%s': moddir='%s', modpath='%s'\n", prog, i, moddir, argv[i], modpath);
    //--
    //modpath = argv[i];
    //printf("%s: argv[%d]='%s': modfile='%s'\n", prog, i, argv[i], modpath);

    //-- open module
    if ( !(mod = g_module_open(modpath,G_MODULE_BIND_LOCAL)) ) {
      g_printerr("%s: could not load module '%s': %s - skipping\n", prog, modpath, g_module_error());
      if (modpath != argv[i]) g_free(modpath);
      continue;
    }
    printf("-> open(): %p\n", mod);
    
    //-- get symbol 'foo' from module
    if (!g_module_symbol(mod,"foo",(gpointer *)&foofunc)) {
      g_printerr("%s: could not load symbol 'foo' from module '%s': %s\n", prog, modpath, g_module_error());
      g_module_close(mod);
      if (modpath != argv[i]) g_free(modpath);
      continue;
    }
    printf("-> symbol('foo'): %p\n", foofunc);

    //-- call 'foo' as a foofunc
    printf("-> calling foo(): ");
    foofunc();

    //-- cleanup
    if (modpath != argv[i]) g_free(modpath);
    g_module_close(mod);
  }
  return 0;
}
