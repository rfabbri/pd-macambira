/* In NT, this is all there is to pd; the rest sits in a "pdlib" dll so
that externs can link back to functions defined in pd. */


int sys_main(int argc, char **argv);

int main(int argc, char **argv)
{
    return (sys_main(argc, argv));
}
