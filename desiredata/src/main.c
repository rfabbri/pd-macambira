/* this file is separate because it is outside of libpd.so */

extern "C" int sys_main(int argc, char **argv);
#if _MSC_VER
#include <windows.h>
#include <stdio.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    __try {
        sys_main(__argc,__argv);
    } __finally {
        printf("caught an exception; stopping\n");
    }
}
#else /* not MSVC */
int main(int argc, char **argv) {return sys_main(argc, argv);}
#endif

