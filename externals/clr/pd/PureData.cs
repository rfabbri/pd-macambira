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

        protected virtual void MethodBang() { throw new System.NotImplementedException("Bang handler not defined"); }

        protected virtual void MethodFloat(float f) { throw new System.NotImplementedException("Float handler not defined"); }

        protected virtual void MethodSymbol(Symbol s) { throw new System.NotImplementedException("Symbol handler not defined"); }

        protected virtual void MethodPointer(Pointer p) { throw new System.NotImplementedException("Pointer handler not defined"); }

        protected virtual void MethodList(AtomList lst) { throw new System.NotImplementedException("List handler not defined"); }

        protected virtual void MethodAnything(Symbol tag,AtomList lst) { throw new System.NotImplementedException("Anything handler not defined"); }
    }
}
