#ifndef __SHAREMEM_H
#define __SHAREMEM_H

#include <windows.h>
#include <memory.h>

//
// allocate named shared memory
//
LPVOID smalloc(HANDLE *h, char *name, DWORD size)
{
	LPVOID p;

	*h = CreateFileMapping(
	 INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
	 size, name
	);

	if(*h==NULL) return(NULL);

	p = MapViewOfFile( *h, FILE_MAP_WRITE, 0, 0, 0);

	if(p==NULL)
	{
		CloseHandle(*h);
		return(NULL);
	}
	return(p);
}

//
// open access to named shared memory
//
LPVOID smopen(HANDLE *h, char *name)
{
	LPVOID p;

	*h = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, name);

	if(*h==NULL) return(NULL);

	p = MapViewOfFile(*h, FILE_MAP_WRITE, 0, 0, 0);

	if(p==NULL)
	{
		CloseHandle(*h);
		return(NULL);
	}
	return(p);
}

//
// cleanup
//
void smfree(HANDLE *h, LPVOID p)
{
	UnmapViewOfFile(p);
	CloseHandle(*h);
}

#endif
