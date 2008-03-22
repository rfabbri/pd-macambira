package com.e1.pdj;

import com.cycling74.max.MaxSystem;
import java.io.File;

class JavacCompiler extends GenericCompiler {

	public JavacCompiler() {
		super("javac");
	}
	
	String javacPath() {
		String fullPath = System.getProperty("pdj.JAVA_HOME");
		File test = new File(new File(fullPath, "bin"), "javac");
		if ( test.exists() ) {
			return test.getAbsolutePath();
		}
        MaxSystem.post("unable to find 'bin/javac' from the JAVA_HOME, using PATH");
		return "javac";
	}
    
	void compileClass() throws PDJClassLoaderException {
		String args = javacPath() + " " + resolvJavaFile() + 
		    " -classpath " + getConfigurationClassPath() + 
			" -sourcepath " + PDJClassLoader.fclasses.toString();
		
		int rc = exec(args);
		
		if ( rc != 0 ) {
			throw new PDJClassLoaderException("pdj: compiler returned: "+ rc + ",args: " +args);
		}
		MaxSystem.post("pdj: compile successful");
	}
}
