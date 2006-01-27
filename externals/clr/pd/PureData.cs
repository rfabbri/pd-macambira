using System;
using System.Runtime.CompilerServices; // for extern import

namespace PureData
{
    // PD core functions
    public class Core 
    {
        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        public extern static void Post(string message);        

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        public extern static void PostError(string message);        

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        public extern static void PostBug(string message);        

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        public extern static void PostVerbose(string message);        

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        public extern static IntPtr GenSym(string sym);        

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        public extern static string EvalSym(Symbol sym);        
    }
    
    // This is the base class for a PD/CLR external
    public class External
        : Core
    {
        private readonly IntPtr ptr;
        
        protected virtual void MethodBang() { Post("No bang handler"); }

        protected virtual void MethodFloat(float f) { Post("No float handler"); }

        protected virtual void MethodSymbol(Symbol s) { Post("No symbol handler"); }

        protected virtual void MethodPointer(Pointer p) { Post("No pointer handler");}

        protected virtual void MethodList(Atom[] lst) { Post("No list handler"); }

        protected virtual void MethodAnything(Atom[] lst) { Post("No list handler"); }
    }
}
