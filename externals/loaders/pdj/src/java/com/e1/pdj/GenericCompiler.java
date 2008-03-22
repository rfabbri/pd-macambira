package com.e1.pdj;

import java.io.*;

import com.cycling74.max.MaxSystem;

abstract class GenericCompiler {
	File javaFile;
	String cp[];
	String compilerName;
	
	static String rtJar;
	
	GenericCompiler(String name) { 
		compilerName = name + ": ";
	}
	
	String getConfigurationClassPath() {
		return PDJClassLoader.getConfigurationClassPath();
	}
	
	File resolvJavaFile() {
		int base = PDJClassLoader.fclasses.toString().length() + 1;
		String className = javaFile.toString().substring(base);
		return new File(className);
	}
	
	private int doexec(String arg) throws Exception {
		Process p = Runtime.getRuntime().exec(arg, null, PDJClassLoader.fclasses);
		
		BufferedReader stdout = new BufferedReader(new InputStreamReader(p.getInputStream()));
		BufferedReader stderr = new BufferedReader(new InputStreamReader(p.getErrorStream()));
		String buff;
		boolean cont = true;
		while(cont) {
			cont = false;
			
			buff = stdout.readLine();
			if ( buff != null ) {
				MaxSystem.post(compilerName + buff);
				cont = true;
			} else {
				cont = false; 
			}
		
			buff = stderr.readLine();
			if ( buff != null ) {
				MaxSystem.post(compilerName + buff);
				cont = true;
			} else {
				cont = cont & false;
			}
		}
		
		p.waitFor();
		
		return p.exitValue();
	}
	
	int exec(String arg) throws PDJClassLoaderException {
		MaxSystem.post("pdj: trying to compile class: " + resolvJavaFile().toString());
		
		try {
			return doexec(arg);
		} catch (Exception e) {
			throw new PDJClassLoaderException(e);
		}
	}
	
	abstract void compileClass() throws PDJClassLoaderException;
}
