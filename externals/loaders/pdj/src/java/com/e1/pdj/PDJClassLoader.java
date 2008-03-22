package com.e1.pdj;

import java.net.URLClassLoader;
import java.net.*;
import java.util.*;
import java.io.*;

import com.cycling74.max.MaxSystem;

public class PDJClassLoader extends URLClassLoader {
    /** The folder where the class (and java files) are */
	static File fclasses;
    
    /** if this is set to true, all classloader operation will be logged */
    static boolean verboseCL = false;
    
	static private PDJClassLoader instance;
    
	static {
	    String prop = System.getProperty("pdj.classes-dir");

	    if ( prop == null ) {
	        fclasses = new File(System.getProperty("pdj.home") + "/classes");
	    } else {
	        fclasses = new File(prop);
	    }
	    instance = new PDJClassLoader();
        
	    verboseCL = PDJSystem.isSystemPropertyTrue("pdj.verbose-classloader");
	}

	static public PDJClassLoader getInstance() {
		return instance;
	}

	public PDJClassLoader() {
		super(resolvClasspath());
	}

    private static void findJars(File f, Collection v) throws MalformedURLException {
        File files[] = f.listFiles();

        for(int i = 0;i<files.length;i++) {
            String filename = files[i].toString(); 
            if ( filename.endsWith(".jar") || filename.endsWith(".zip") ) {
				v.add(new URL("jar:" + files[i].toURL().toString() + "!/"));
            }
        }
    }

	private static URL[] resolvClasspath() {
		String cp = System.getProperty("pdj.classpath");
		ArrayList list = new ArrayList();
		String pathSeparator = "" + File.pathSeparator;
        
        try {
            if ( fclasses.isDirectory() )
                list.add(fclasses.toURL());
            
            File lib = new File(System.getProperty("pdj.home") + "/lib");
            if ( lib.isDirectory() )
                findJars(lib, list);
            
            StringTokenizer st = new StringTokenizer(cp, pathSeparator);
            while( st.hasMoreTokens() ) {
                String token = st.nextToken(pathSeparator);
                File f = new File(token);
                if ( f.isFile() && (f.toString().endsWith(".jar") || f.toString().endsWith(".zip")) )
                    list.add(new URL("jar:" + f.toURL().toString() + "!/"));
                if ( f.isDirectory() ) {
                    findJars(f, list);
                    list.add(f.toURL());
                }
	        }
        } catch ( MalformedURLException mue ) {
            throw new Error("pdj: unable to add to classpath: " + mue);
        }
        
        URL ret[] = (URL[]) list.toArray(new URL[list.size()]);
        
        if ( verboseCL ) {
            MaxSystem.post("pdj: verbose classloader: system classpath: " + System.getProperty("java.class.path"));
            MaxSystem.post("pdj: verbose classloader: dynamic classpath:");
            for(int i=0;i<ret.length;i++) {
                MaxSystem.post("\t" + ret[i].toString());
            }
        }

		return ret;
	}

	public static String[] getCurrentClassPath() {
		URL url[] = instance.getURLs();
		String ret[] = new String[url.length];

		for(int i=0;i<url.length;i++) {
			ret[i] = url[i].toString();
		}
		return ret;
	}

	public static String getReadableClassPath() {
		PDJClassLoader cloader = getInstance();
		StringBuffer sb = new StringBuffer();

		URL url[] = cloader.getURLs();
		for(int i=0;i<url.length;i++) {
			sb.append("\t" + url[i].toString() + "\n");
		}

		return sb.toString();
	}

	/**
	 * Returns a readable classpath for javac
	 * @return the classpath for javac / jikes
	 */
	public static String getConfigurationClassPath() {
		StringBuffer sb = new StringBuffer();

		sb.append(System.getProperty("java.class.path") + File.pathSeparatorChar);

		URL url[] = getInstance().getURLs();
		for(int i=0;i<url.length;i++) {
			String path = url[i].toString();

			if ( path.startsWith("file:") ) { 
			    sb.append(path.substring(6) + File.pathSeparatorChar);
                continue;
            }
            
            if ( path.startsWith("jar:") ) {
                sb.append(path.substring(10, path.lastIndexOf("!")) + File.pathSeparatorChar);
                continue;
            }
                
			sb.append(path + File.pathSeparatorChar);
		}

		return sb.toString();
	}

	public static void resetClassloader() {
		instance = new PDJClassLoader();
		System.gc();
	}

	/**
	 * Dynamic resolv will try to load .java file from date with .class
	 * @param name classname
	 * @return class definition
	 * @throws ClassNotFoundException
	 */
	public static Class dynamicResolv(String name) throws PDJClassLoaderException {
        if ( System.getProperty("pdj.compiler").equals("null") ) {
            resetClassloader();
            try {
                return instance.loadClass(name);
            } catch (ClassNotFoundException e) {
                throw new PDJClassLoaderException(e);
            }
        }

		String pathName = name.replace('.', File.pathSeparatorChar);

		File classFile = new File(fclasses, pathName + ".class");
		File javaFile = new File(fclasses, pathName + ".java");

		if ( javaFile.exists() ) {
            if ( classFile.exists() ) {
                if ( javaFile.lastModified() > classFile.lastModified() ) {
                    compileClass(javaFile);
                } else {
                    if ( verboseCL )
                        MaxSystem.post("class: " + name + " is already compiled and younger than the source .java file");
                }
            } else {
                compileClass(javaFile);
            }
		}

        resetClassloader();                
        
		try {
			return instance.loadClass(name);
		} catch (ClassNotFoundException e) {
			throw new PDJClassLoaderException(e);
		}
	}

	protected static void compileClass(File javaFile) throws PDJClassLoaderException {
		String str_compiler = System.getProperty("pdj.compiler");
		GenericCompiler compiler = null;

		if ( str_compiler.equals("jikes") ) {
			compiler = new JikesCompiler();
		} else {
            if ( !str_compiler.equals("javac") ) {
                System.err.println("pdj: unknown compiler:" + str_compiler + ", using javac.");
            }
            compiler = new JavacCompiler();
        }
		compiler.javaFile = javaFile;
		compiler.compileClass();
	}
}
