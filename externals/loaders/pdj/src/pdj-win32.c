#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "pdj.h"

int getuglylibpath(char *path) {
	HMODULE hmodule = GetModuleHandle("pdj.dll");
	char dllPath[BUFFER_SIZE];
	int rc;
	
	if ( hmodule == NULL ) {
		post("pdj: can't get windows dll handle");
		strcpy(path, ".");
		return 1;
	}
		
	rc = GetModuleFileName(hmodule, dllPath, 1023);
	if ( rc == 0 || rc == 1023 ) {
		post("pdj: can't get windows dll path");
		strcpy(path, ".");
		return 1;
	}
	
	dllPath[strlen(dllPath)-8] = 0;
	strcpy(path, dllPath);
	return 0;
}


static char *getJavaHomeFromReg(char *javaHome) {
	char keyName[BUFFER_SIZE] = "Software\\JavaSoft\\Java Development Kit";
	char *dest;
	int size = BUFFER_SIZE;
	HKEY hKey;
	long rc;

	rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyName, 0, KEY_READ, &hKey);
	if ( rc != ERROR_SUCCESS )
		return NULL;

	strcat(keyName, "\\");
	dest = strlen(keyName) + keyName;
	rc = RegQueryValueEx(hKey, "CurrentVersion", NULL, NULL, 
		(LPBYTE) dest, &size);

	RegCloseKey(hKey);

	if ( rc != ERROR_SUCCESS )
		return NULL;

	rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyName, 0, KEY_READ, &hKey);
	if ( rc != ERROR_SUCCESS )
		return NULL;

	size = BUFFER_SIZE;
	rc = RegQueryValueEx(hKey, "JavaHome", NULL, NULL, 
		(LPBYTE) javaHome, &size);

	RegCloseKey(hKey);

	if ( rc != ERROR_SUCCESS ) 
		return NULL;

	return javaHome;
}


JNI_CreateJavaVM_func *linkjvm(char *vm_type) {
	HINSTANCE jvmDll = (HINSTANCE)NULL;
	JNI_CreateJavaVM_func *func;
	char work[BUFFER_SIZE];
	char *javahome = pdj_getProperty("pdj.JAVA_HOME");
	void *libVM;

	// default config or no config ? try 
	if ( javahome == NULL || javahome[0] == '/' ) {
		javahome = NULL;
		javahome = getenv("JAVA_HOME");
		if ( javahome == NULL ) {
			javahome = getJavaHomeFromReg(work);
			if ( javahome == NULL ) {
				post("pdj: unable to find any java VM. Please set JAVA_HOME environment variable");
				return NULL;
			}
		}
	}

	strcpy(work, javahome);
	strcat(work, "\\jre\\bin\\");
	strcat(work, vm_type);
	strcat(work, "\\jvm.dll");
	jvmDll = LoadLibrary(work);

	if ( jvmDll == NULL ) {
		post("post: unable to find jvm.dll in %s", work);
		return NULL;
	}
	func = GetProcAddress(jvmDll, "JNI_CreateJavaVM");
	if ( func == NULL ) {
		post("pdj: unable to find symbol: JNI_CreateJavaVM in jvm.dll ??");
		return NULL;
	}

	post("pdj: using JVM %s", javahome);

	return func;
}
