/* Copyright (c) 1999 Guenter Geiger and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This file implements the loader for linux, which includes a little bit of path handling.
 * Generalized by MSP to provide an open_via_path function and lists of files for all purposes. */
/* #define DEBUG(x) x */
#define DEBUG(x)

#include <stdlib.h>
#ifdef UNISTD
#include <unistd.h>
#include <sys/stat.h>
#endif
#ifdef MSW
#include <io.h>
#endif

#include <string.h>
#include "desire.h"
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <vector>

extern t_namelist *sys_externlist;
t_namelist *sys_searchpath;
t_namelist *sys_helppath;

/* change '/' characters to the system's native file separator */
void sys_bashfilename(const char *from, char *to) {
    char c;
    while ((c = *from++)) {
#ifdef MSW
        if (c == '/') c = '\\';
#endif
        *to++ = c;
    }
    *to = 0;
}

/* change the system's native file separator to '/' characters  */
void sys_unbashfilename(const char *from, char *to) {
    char c;
    while ((c = *from++)) {
#ifdef MSW
        if (c == '\\') c = '/';
#endif
        *to++ = c;
    }
    *to = 0;
}

/*******************  Utility functions used below ******************/

/* copy until delimiter and return position after delimiter in string */
/* if it was the last substring, return NULL */

static const char *strtokcpy(char *&to, const char *from, int delim) {
    int size = 0;
    while (from[size] != (char)delim && from[size] != '\0') size++;
    to = (char *)malloc(size+1);
    strncpy(to,from,size);
    to[size] = '\0';
    if (from[size] == '\0') return NULL;
    return size ? from+size+1 : 0;
}

/* add a single item to a namelist.  If "allowdup" is true, duplicates
may be added; othewise they're dropped.  */
t_namelist *namelist_append(t_namelist *listwas, const char *s, int allowdup) {
    t_namelist *nl, *nl2 = (t_namelist *)getbytes(sizeof(*nl));
    nl2->nl_next = 0;
    nl2->nl_string = strdup(s);
    sys_unbashfilename(nl2->nl_string, nl2->nl_string);
    if (!listwas) return nl2;
    for (nl = listwas; ;) {
        if (!allowdup && !strcmp(nl->nl_string, s)) return listwas;
        if (!nl->nl_next) break;
        nl = nl->nl_next;
    }
    nl->nl_next = nl2;
    return listwas;
}

/* add a colon-separated list of names to a namelist */

#ifdef MSW
#define SEPARATOR ';'   /* in MSW the natural separator is semicolon instead */
#else
#define SEPARATOR ':'
#endif

t_namelist *namelist_append_files(t_namelist *listwas, const char *s) {
    const char *npos = s;
    t_namelist *nl = listwas;
    do {
        char *temp;
        npos = strtokcpy(temp, npos, SEPARATOR);
        if (!*temp) continue;
        nl = namelist_append(nl, temp, 0);
	free(temp);
    } while (npos);
    return nl;
}

void namelist_free(t_namelist *listwas) {
    t_namelist *nl2;
    for (t_namelist *nl = listwas; nl; nl = nl2) {
        nl2 = nl->nl_next;
        free(nl->nl_string);
        free(nl);
    }
}

char *namelist_get(t_namelist *namelist, int n) {
    int i=0;
    for (t_namelist *nl = namelist; i < n && nl; nl = nl->nl_next) {if (i==n) return nl->nl_string; else i++;}
    return 0;
}

static t_namelist *pd_extrapath;

int sys_usestdpath = 1;

void sys_setextrapath(const char *p) {
    namelist_free(pd_extrapath);
    pd_extrapath = namelist_append(0, p, 0);
}

#ifdef MSW
#define MSWOPENFLAG(bin) (bin ? _O_BINARY : _O_TEXT)
#else
#define MSWOPENFLAG(bin) 0
#endif

/* try to open a file in the directory "dir", named "name""ext", for reading. "Name" may have slashes.
   The directory is copied to "dirresult" which must be at least "size" bytes.  "nameresult" is set
   to point to the filename (copied elsewhere into the same buffer). The "bin" flag requests opening
   for binary (which only makes a difference on Windows). */
int sys_trytoopenone(const char *dir, const char *name, const char* ext, char **dirresult, char **nameresult, int bin) {
    bool needslash = (*dir && dir[strlen(dir)-1] != '/');
    asprintf(dirresult,"%s%s%s%s", dir, needslash ? "/" : "", name, ext);
    sys_bashfilename(*dirresult, *dirresult);
    DEBUG(post("looking for %s",*dirresult));
    /* see if we can open the file for reading */
    int fd = open(*dirresult,O_RDONLY | MSWOPENFLAG(bin));
    if (fd<0) {
        if (sys_verbose) post("tried %s and failed", *dirresult);
        return -1;
    }
#ifdef UNISTD /* in unix, further check that it's not a directory */
    struct stat statbuf;
    int ok =  (fstat(fd, &statbuf) >= 0) && !S_ISDIR(statbuf.st_mode);
    if (!ok) {
        if (sys_verbose) post("tried %s; stat failed or directory", *dirresult);
        close (fd);
        return -1;
    }
#endif
    if (sys_verbose) post("tried %s and succeeded", *dirresult);
    sys_unbashfilename(*dirresult, *dirresult);
    char *slash = strrchr(*dirresult, '/');
    if (slash) {
        *slash = 0;
        *nameresult = slash + 1;
    } else *nameresult = *dirresult;
    return fd;
}

/* check if we were given an absolute pathname, if so try to open it and return 1 to signal the caller to cancel any path searches */
int sys_open_absolute(const char *name, const char* ext, char **dirresult, char **nameresult, int bin, int *fdp) {
    if (name[0] == '/'
#ifdef MSW
        || (name[1] == ':' && name[2] == '/')
#endif
    ) {
        int dirlen = strrchr(name, '/') - name;
        char *dirbuf = new char[dirlen+1];
        *fdp = sys_trytoopenone(name, name+dirlen+1, ext, dirresult, nameresult, bin);
	delete[] dirbuf;
        return 1;
    } else return 0;
}

/* search for a file in a specified directory, then along the globally
defined search path, using ext as filename extension.  The
fd is returned, the directory ends up in the "dirresult" which must be at
least "size" bytes.  "nameresult" is set to point to the filename, which
ends up in the same buffer as dirresult.  Exception:
if the 'name' starts with a slash or a letter, colon, and slash in MSW,
there is no search and instead we just try to open the file literally.  */

/* see also canvas_openfile() which, in addition, searches down the
canvas-specific path. */

static int do_open_via_path(
const char *dir, const char *name, const char *ext, char **dirresult, char **nameresult, int bin, t_namelist *searchpath) {
    int fd = -1;
    /* first check if "name" is absolute (and if so, try to open) */
    if (sys_open_absolute(name, ext, dirresult, nameresult, bin, &fd)) return fd;
    /* otherwise "name" is relative; try the directory "dir" first. */
    if ((fd = sys_trytoopenone(dir, name, ext, dirresult, nameresult, bin)) >= 0) return fd;
    /* next go through the search path */
    for (t_namelist *nl=searchpath; nl; nl=nl->nl_next)
        if ((fd = sys_trytoopenone(nl->nl_string, name, ext, dirresult, nameresult, bin)) >= 0) return fd;
    /* next look in "extra" */
    if (sys_usestdpath && (fd = sys_trytoopenone(pd_extrapath->nl_string, name, ext, dirresult, nameresult, bin)) >= 0)
                return fd;
    *dirresult = 0;
    *nameresult = *dirresult;
    return -1;
}

extern "C" int open_via_path2(const char *dir, const char *name, const char *ext, char **dirresult, char **nameresult, int bin) {
    return do_open_via_path(dir, name, ext, dirresult, nameresult, bin, sys_searchpath);
}

/* open via path, using the global search path. */
extern "C" int open_via_path(const char *dir, const char *name, const char *ext,
char *dirresult, char **nameresult, unsigned int size, int bin) {
    char *dirr;
    int r = do_open_via_path(dir, name, ext, &dirr, nameresult, bin, sys_searchpath);
    if (dirr) {strncpy(dirresult,dirr,size); dirresult[size-1]=0; free(dirr);}
    return r;
}

/* Open a help file using the help search path.  We expect the ".pd" suffix here,
   even though we have to tear it back off for one of the search attempts. */
extern "C" void open_via_helppath(const char *name, const char *dir) {
    char *realname=0, *dirbuf, *basename;
    int suffixed = strlen(name) > 3 && !strcmp(name+strlen(name)-3, ".pd");
    asprintf(&realname,"%.*s-help.pd",strlen(name)-3*suffixed,name);
    int fd;
    if ((fd = do_open_via_path(dir,realname,"",&dirbuf,&basename,0,sys_helppath))>=0) goto gotone;
    free(realname);
    asprintf(&realname,"help-%s",name);
    if ((fd = do_open_via_path(dir,realname,"",&dirbuf,&basename,0,sys_helppath))>=0) goto gotone;
    free(realname);
    if ((fd = do_open_via_path(dir,    name,"",&dirbuf,&basename,0,sys_helppath))>=0) goto gotone;
    post("sorry, couldn't find help patch for \"%s\"", name);
    return;
gotone:
    close(fd); if (realname) free(realname);
    glob_evalfile(0, gensym((char*)basename), gensym(dirbuf));
}

extern "C" int sys_argparse(int argc, char **argv);

#define NUMARGS 1000
#define foreach(ITER,COLL) for(typeof(COLL.begin()) ITER = COLL.begin(); ITER != (COLL).end(); ITER++)

extern "C" int sys_parsercfile(char *filename) {
    std::vector<char*> argv;
    char buf[1000];
    char c[MAXPDSTRING];
    int retval = 1; /* that's what we will return at the end; for now, let's think it'll be an error */
    /* parse a startup file */
    FILE* file = fopen(filename, "r");
    if (!file) return 1;
    post("reading startup file: %s", filename);
    /* tb originally introduced comments in pdrc file. desire.tk doesn't support them. */
    while ((fgets(c,MAXPDSTRING,file)) != 0) {
	if (c[strlen(c)-1] !='\n') {
		error("startup file contains a line that's too long");
		while(fgetc(file) != '\n') {}
	}
	if (c[0] != '#') {
		long j=0;
		long n;
		while (sscanf(c+j,"%999s%ln",buf,&n) != EOF) {argv.push_back(strdup(buf)); j+=n;}
	}
    }
    /* parse the options */
    fclose(file);
    if (sys_verbose) {
        if (argv.size()) {
            post("startup args from RC file:");
            foreach(a,argv) post("%s",*a);
        } else post("no RC file arguments found");
    }
    if (sys_argparse(argv.size(),argv.data())) {
        post("error parsing RC arguments");
	goto cleanup;
    }
    retval=0; /* we made it without an error */
  cleanup: /* prevent memleak */
    foreach(a,argv) free(*a);
    return retval;
}

#ifndef MSW
#define STARTUPNAME ".pdrc"
extern "C" int sys_rcfile () {
    char *fname, *home = getenv("HOME");
    asprintf(&fname,"%s/%s",home? home : ".",STARTUPNAME);
    int r = sys_parsercfile(fname);
    free(fname);
    return r;
}
#endif /* MSW */

void sys_doflags() {
    int beginstring = 0, state = 0, len = strlen(sys_flags->s_name);
    int rcargc = 0;
    char *rcargv[MAXPDSTRING];
    if (len > MAXPDSTRING) {post("flags: %s: too long", sys_flags->s_name); return;}
    for (int i=0; i<len+1; i++) {
        int c = sys_flags->s_name[i];
        if (state == 0) {
            if (c && !isspace(c)) {
                beginstring = i;
                state = 1;
            }
        } else {
            if (!c || isspace(c)) {
                char *foo = (char *)malloc(i - beginstring + 1);
                if (!foo) return;
                strncpy(foo, sys_flags->s_name + beginstring, i - beginstring);
                foo[i - beginstring] = 0;
                rcargv[rcargc] = foo;
                rcargc++;
                if (rcargc >= MAXPDSTRING) break;
                state = 0;
            }
        }
    }
    if (sys_argparse(rcargc, rcargv)) post("error parsing startup arguments");
}

extern "C" void glob_update_path () {
    t_namelist *nl;
    sys_vgui("global pd_path; set pd_path {");
    for (nl=sys_searchpath; nl; nl=nl->nl_next) sys_vgui("%s ",nl->nl_string);
    sys_vgui("}\n");
}
