#include <dlfcn.h>

void scaf_feeder_asm (void *tos, void *reg, void (*ca_rule)(), void *env);

void ca_test() {}

main()
{
    int stack[256];
    int reg[8];
    int env[8];

    void *libhandle;
    void *ca_routine;


    if (!(libhandle = dlopen("../modules/test.scafo", RTLD_NOW))){
	printf("error: %s\n", dlerror());
	exit(1);
    }
    
    if (!(ca_routine = dlsym(libhandle, "carule_1"))){
	printf("error: %s\n", dlerror());
	exit(1);
    }

    scaf_feeder_asm(stack+254, reg, ca_routine, env);

    dlclose(libhandle);
}
