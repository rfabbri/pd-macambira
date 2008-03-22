package com.e1.pdj;

import com.cycling74.max.MaxSystem;

public class JikesCompiler extends GenericCompiler {

	JikesCompiler() {
		super("jikes");
	}
	
	void compileClass() throws PDJClassLoaderException {
		if ( GenericCompiler.rtJar == null )
			throw new PDJClassLoaderException("pdj: JAVA_HOME not found");

		String args = "jikes " + resolvJavaFile() + 
	    " -classpath " + GenericCompiler.rtJar + getConfigurationClassPath() + 
		" -sourcepath " + PDJClassLoader.fclasses.toString();
	
		int rc = exec(args);
	
		if ( rc != 0 ) {
			throw new PDJClassLoaderException("pdj: compiler returned: "+ rc + ",args: " +args);
		}
		MaxSystem.post("pdj: compile successful");
	}
}
