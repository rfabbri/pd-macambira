/* In MSW, this is all there is to pd; the rest sits in a "pdlib" dll so
that externs can link back to functions defined in pd. */

int sys_main(int argc, char **argv);

#ifdef MSW
#include <windows.h>
#include <stdio.h>

int WINAPI WinMain(HINSTANCE hInstance,
                               HINSTANCE hPrevInstance,
                               LPSTR lpCmdLine,
                               int nCmdShow)
{ 
    __try {
        sys_main(__argc,__argv);
    }
    __finally
    { 
        printf("caught an exception; stopping\n");
    }
}

#else /* not MSW */
int main(int argc, char **argv)
{
    return (sys_main(argc, argv));
}
#endif


