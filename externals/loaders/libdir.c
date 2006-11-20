#include "m_pd.h"
#include "s_stuff.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char *version = "$Revision: 1.2 $";

/* This loader opens a directory as a library.  In the long run, the idea is
 * that one folder will have all of objects files, all of the related
 * *-help.pd files, a file with meta data for the help system, etc.  Then to
 * install the lib, it would just be dropped into extra, or the path anywhere.
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
    char searchpathname[MAXPDSTRING], helppathname[MAXPDSTRING], 
        fullclassname[MAXPDSTRING], dirbuf[MAXPDSTRING], *nameptr;
    
    post("libdir_loader classname: %s\n", classname);

    /* look for meta file (classname)/(classname)-meta.pd <hans@at.or.at> */
    /* TODO: add "-META" to the meta filename */
    strncpy(fullclassname, classname, MAXPDSTRING - 6);
    strcat(fullclassname, "/");
    strncat(fullclassname, classname, MAXPDSTRING - strlen(fullclassname) - 6);
    strcat(fullclassname, "-meta");
	post("libdir_loader trying fullclassname: '%s'\n", fullclassname);
//	post("patch dir: '%s'",canvas->gl_env->ce_dir->s_name);
	if ((fd = open_via_path("", fullclassname, ".pd",
							dirbuf, &nameptr, MAXPDSTRING, 1)) < 0) 
    {
        return (0);
	}
    close(fd);
    post("libdir_loader loaded fullclassname: '%s'\n", fullclassname);
    
        /* create full path to libdir for adding to the paths */
    strcpy(searchpathname,dirbuf);
//    strcat(searchpathname,"/");
//    strncat(searchpathname,classname, MAXPDSTRING-strlen(searchpathname));

    strncpy(helppathname, sys_libdir->s_name, MAXPDSTRING-30);
    helppathname[MAXPDSTRING-30] = 0;
    strcat(helppathname, "/doc/5.reference/");
    strcat(helppathname, classname);

	// TODO: have this add to the canvas-local path only
    sys_searchpath = namelist_append_files(sys_searchpath, searchpathname);
    /* this help path supports having the help files in a complete library
     * directory format, where everything is in one folder.  The help meta
     * system needs to be implemented for this to work with the new Help
     * menu/browser system. Ultimately, /path/to/extra will have to be added
     * to sys_helppath in order for this to work properly.<hans@at.or.at> */
    sys_helppath = namelist_append_files(sys_helppath, searchpathname);
    
        /* this should be changed to use sys_debuglevel */
    if (sys_verbose) 
    {
        post("Added to search path: %s", searchpathname);
        post("Added to help path: %s", searchpathname);
        post("Added to help path: %s", helppathname);
    }
    if (sys_verbose) 
        post("Loaded libdir %s from %s", classname, dirbuf);

    return (1);
}

void libdir_setup(void)
{
    sys_register_loader(libdir_loader);
    post("libdir loader %s",version);  
    post("\twritten by Hans-Christoph Steiner <hans@at.or.at>");
    post("\tcompiled on "__DATE__" at "__TIME__ " ");
}
