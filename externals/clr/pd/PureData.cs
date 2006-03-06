using System;
using System.Runtime.CompilerServices; // for extern import
using System.Runtime.InteropServices; // for structures

namespace PureData
{
    // PD core functions
    public unsafe class Core 
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
        internal extern static void *SymGen(string sym);        

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static string SymEval(void *sym);        
    }
    
    // This is the base class for a PD/CLR external
    public unsafe class External
        : Core
    {
        // PD object pointer
        private readonly void *ptr;

        protected virtual void MethodBang() { Post("No bang handler"); }

        protected virtual void MethodFloat(float f) { Post("No float handler"); }

        protected virtual void MethodSymbol(Symbol s) { Post("No symbol handler"); }

        protected virtual void MethodPointer(Pointer p) { Post("No pointer handler");}

        protected virtual void MethodList(AtomList lst) { Post("No list handler"); }

        protected virtual void MethodAnything(Symbol tag,AtomList lst) { Post("No anything handler"); }
    }
}
