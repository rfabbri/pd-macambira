/* In MSW, this is all there is to pd; the rest sits in a "pdlib" dll so
that externs can link back to functions defined in pd. */

#include <stdio.h>

int sys_main(int argc, char **argv);

#ifdef MSW
#include <windows.h>
#include <stdio.h>
#include <malloc.h>

#define MAXARGS 1024
#define MAXARGLEN 1024

/* jsarlo { */
int tokenizeCommandLineString(char *clString, char **tokens)
{
    int i, charCount = 0;
    int tokCount= 0;
    int quoteOpen = 0;

    for (i = 0; i < (int)strlen(clString); i++)
    {
        if (clString[i] == '"')
        {
            quoteOpen = !quoteOpen;
        }
        else if (clString[i] == ' ' && !quoteOpen)
        {
            tokens[tokCount][charCount] = 0;
            tokCount++;
            charCount = 0;
        }
        else
        {
            tokens[tokCount][charCount] = clString[i];
            charCount++;
        }
    }
    tokens[tokCount][charCount] = 0;
    tokCount++;
    return tokCount;
}

int WINAPI WinMain(HINSTANCE hInstance, 
                               HINSTANCE hPrevInstance,
                               LPSTR lpCmdLine,
                               int nCmdShow)
{
    int i, argc;
    char *argv[MAXARGS];

     __try
    {
        for (i = 0; i < MAXARGS; i++)
        {
            argv[i] = (char *)malloc(MAXARGLEN * sizeof(char));
        }
        GetModuleFileName(NULL, argv[0], MAXARGLEN);
        argc = tokenizeCommandLineString(lpCmdLine, argv + 1) + 1;
        sys_main(argc, argv);
        for (i = 0; i < MAXARGS; i++)
        {
            free(argv[i]);
        }
    }
    __finally
    {
        printf("caught an exception; stopping\n");
    }
}

/* } jsarlo */

#else /* not MSW */
int main(int argc, char **argv)
{
    return (sys_main(argc, argv));
}
#endif


