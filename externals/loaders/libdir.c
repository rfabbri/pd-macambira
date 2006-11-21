#include "m_pd.h"
#include "s_stuff.h"
#include "g_canvas.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* WARNING: KLUDGE!  */
/*
 * this struct is not publically defined (its in g_canvas.c) so I need to
 * include this here.  Its from Pd 0.41-test03 2006-11-19. */
struct _canvasenvironment
{
    t_symbol *ce_dir;      /* directory patch lives in */
    int ce_argc;           /* number of "$" arguments */
    t_atom *ce_argv;       /* array of "$" arguments */
    int ce_dollarzero;     /* value of "$0" */
    t_namelist *ce_path;   /* search path */
};


static char *version = "$Revision: 1.3 $";

/* This loader opens a directory with a -meta.pd file as a library.  In the
 * long run, the idea is that one folder will have all of objects files, all
 * of the related *-help.pd files, a file with meta data for the help system,
 * etc.  Then to install the lib, it would just be dropped into extra, or
 * anywhere in the global classpath.
 *
 * Ultimately, the meta file will be read for meta data, specifically for
 * the auto-generated Help system, but for other things too.  Right now,
 * its just used as a marker that a directory is meant to be a library.
 * Plus its much easier to implement it this way, I can use
 * open_via_path() instead of writing a new function.  The grand plan is
 * to have one directory hold the objects, help files, manuals,
 * etc. making it a self-contained library. <hans@at.or.at>
 */

static int libdir_loader(t_canvas *canvas, char *classname)
{
    int fd = -1;
    char fullclassname[MAXPDSTRING], dirbuf[MAXPDSTRING], *nameptr;
	t_canvasenvironment *canvasenvironment;

/* look for meta file (classname)/(classname)-meta.pd */
	strncpy(fullclassname, classname, MAXPDSTRING - 6);
	strcat(fullclassname, "/");
	strncat(fullclassname, classname, MAXPDSTRING - strlen(fullclassname) - 6);
	strcat(fullclassname, "-meta");
	
	/* if this is being called from a canvas, then add the library path to the
	 * canvas-local path */
	if(canvas) 
	{
		post("libdir_loader: adding %s to the canvas-local path", classname);
		canvasenvironment = canvas_getenv(canvas);
		if ((fd = canvas_open(0, fullclassname, ".pd",
								dirbuf, &nameptr, MAXPDSTRING, 0)) < 0) 
		{
			return (0);
		}
		close(fd);
		// TODO: have this add to the canvas-local path only
		canvasenvironment->ce_path = namelist_append(canvasenvironment->ce_path, 
													 dirbuf, 0);
	}
	else
	{
		post("libdir_loader: adding %s to the global classpath", classname);
		post("\tThis is deprecated behavior.");
		if ((fd = open_via_path("", fullclassname, ".pd",
								dirbuf, &nameptr, MAXPDSTRING, 0)) < 0) 
		{
			return (0);
		}
		close(fd);
		sys_searchpath = namelist_append(sys_searchpath, dirbuf, 0);
	}
	/* post("libdir_loader loaded fullclassname: '%s'\n", fullclassname); */
    if (sys_verbose) 
        post("Loaded libdir %s from %s", classname, dirbuf);

    return (1);
}

void libdir_setup(void)
{
/* relies on t.grill's loader functionality, fully added in 0.40 */
#if (PD_MINOR_VERSION >= 40)
    sys_register_loader(libdir_loader);
#else
	error("to function, this needs to be compiled against Pd 0.40 or higher,\n");
	post("\tor a version that has sys_register_loader()");
#endif
    post("libdir loader %s",version);  
    post("\twritten by Hans-Christoph Steiner <hans@at.or.at>");
    post("\tcompiled on "__DATE__" at "__TIME__ " ");
	post("\tcompiled against Pd version %d.%d.%d.%s", PD_MAJOR_VERSION, 
		 PD_MINOR_VERSION, PD_BUGFIX_VERSION, PD_TEST_VERSION);
}
