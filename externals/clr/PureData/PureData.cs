using System;
using System.Runtime.CompilerServices; // for extern import
using System.Runtime.InteropServices; // for structures

namespace PureData
{
    // PD core functions
    public unsafe class Internal 
    {
        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void *SymGen(string sym);        

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static string SymEval(void *sym);        
    }
    
    // This is the base class for a PD/CLR external
    public unsafe class External
    {
        // PD object pointer
        private readonly void *ptr;

        // --------------------------------------------------------------------------

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Post(string message);        

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void PostError(string message);        

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void PostBug(string message);        

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void PostVerbose(string message);        

        // --------------------------------------------------------------------------

        protected delegate void MethodBang();
        protected delegate void MethodFloat(float f);
        protected delegate void MethodSymbol(Symbol s);
        protected delegate void MethodPointer(Pointer p);
        protected delegate void MethodList(AtomList lst);
        protected delegate void MethodAnything(Symbol tag,AtomList lst);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Add(MethodBang m);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Add(MethodFloat m);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Add(MethodSymbol m);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Add(MethodPointer m);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Add(MethodList m);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Add(MethodAnything m);
    }
}
