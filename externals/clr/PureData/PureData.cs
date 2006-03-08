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

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void AddInlet(void *obj,Symbol sel,Symbol to_sel);

        // map to data member
        // \NOTE don't know if this is really safe since f must stay at its place (but it should, no?)
        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void AddInlet(void *obj,ref float f); 

        // map to data member
        // \NOTE don't know if this is really safe since s must stay at its place (but it should, no?)
        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void AddInlet(void *obj,ref Symbol s);

        // map to data member
        // \NOTE don't know if this is really safe since s must stay at its place (but it should, no?)
//        [MethodImplAttribute (MethodImplOptions.InternalCall)]
//        internal extern static void AddInlet(void *obj,ref Pointer f);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void AddInlet(void *obj); // create proxy inlet

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void AddOutlet(void *obj,Symbol type);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void Outlet(void *obj,int n);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void Outlet(void *obj,int n,float f);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void Outlet(void *obj,int n,Symbol s);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void Outlet(void *obj,int n,Pointer p);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void Outlet(void *obj,int n,Atom a);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void Outlet(void *obj,int n,Symbol s,AtomList l);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void Outlet(void *obj,int n,Symbol s,Atom[] l);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void Bind(void *obj,Symbol dst);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void Unbind(void *obj,Symbol dst);
    }
    
    // This is the base class for a PD/CLR external
    public unsafe abstract class External
    {
        // PD object pointer
        private readonly void *ptr;

        // to be returned by Setup function
        protected enum ClassType { Default = 0,PD = 1,GObj = 2,Patchable = 3,NoInlet = 8 }

        // --------------------------------------------------------------------------

        protected readonly static Symbol s__ = new Symbol("");
        protected readonly static Symbol s_bang = new Symbol("bang");
        protected readonly static Symbol s_float = new Symbol("float");
        protected readonly static Symbol s_symbol = new Symbol("symbol");
        protected readonly static Symbol s_pointer = new Symbol("pointer");
        protected readonly static Symbol s_list = new Symbol("list");
        protected readonly static Symbol s_anything = new Symbol("anything");

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
        protected extern static void Add(Symbol sel,MethodList m);

        protected static void Add(string sel,MethodList m) { Add(new Symbol(sel),m); }

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Add(MethodAnything m);

        // --------------------------------------------------------------------------

        protected void AddInlet(Symbol sel,Symbol to_sel) { Internal.AddInlet(ptr,sel,to_sel); }
        protected void AddInlet(ref float f) { Internal.AddInlet(ptr,ref f); }
        protected void AddInlet(ref Symbol s) { Internal.AddInlet(ptr,ref s); }
//        protected void AddInlet(ref Pointer p) { Internal.AddInlet(ptr,ref p); } // map to data member
        protected void AddInlet() { Internal.AddInlet(ptr); } // create proxy inlet

        // --------------------------------------------------------------------------

        protected void AddOutlet(Symbol type) { Internal.AddOutlet(ptr,type); }

        protected void AddOutletBang() { AddOutlet(s_bang); }
        protected void AddOutletFloat() { AddOutlet(s_float); }
        protected void AddOutletSymbol() { AddOutlet(s_symbol); }
        protected void AddOutletPointer() { AddOutlet(s_pointer); }
        protected void AddOutletList() { AddOutlet(s_list); }
        protected void AddOutletAnything() { AddOutlet(s_anything); }

        // --------------------------------------------------------------------------

        protected void Outlet(int n) { Internal.Outlet(ptr,n); }
        protected void Outlet(int n,float f) { Internal.Outlet(ptr,n,f); }
        protected void Outlet(int n,Symbol s) { Internal.Outlet(ptr,n,s); }
        protected void Outlet(int n,Pointer p) { Internal.Outlet(ptr,n,p); }
        protected void Outlet(int n,Atom a) { Internal.Outlet(ptr,n,a); }
        protected void Outlet(int n,AtomList l) { Internal.Outlet(ptr,n,s_list,l); }
        protected void Outlet(int n,Atom[] l) { Internal.Outlet(ptr,n,s_list,l); }
        protected void Outlet(int n,Symbol s,AtomList l) { Internal.Outlet(ptr,n,s,l); }
        protected void Outlet(int n,Symbol s,Atom[] l) { Internal.Outlet(ptr,n,s,l); }

        // --------------------------------------------------------------------------

        // bind to symbol
        protected void Bind(Symbol sym) { Internal.Bind(ptr,sym); }
        protected void Unbind(Symbol sym) { Internal.Unbind(ptr,sym); }

        // send to receiver symbol
        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Send(Symbol sym,Atom a);

        protected static void Send(Symbol sym) { Send(sym,s_bang,null); }
        protected static void Send(Symbol sym,float f) { Send(sym,new Atom(f)); }
        protected static void Send(Symbol sym,Symbol s) { Send(sym,new Atom(s)); }
        protected static void Send(Symbol sym,Pointer p) { Send(sym,new Atom(p)); }
        protected static void Send(Symbol sym,AtomList l) { Send(sym,s_list,l); }
        protected static void Send(Symbol sym,Atom[] l) { Send(sym,s_list,l); }

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Send(Symbol sym,Symbol s,AtomList l);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Send(Symbol sym,Symbol s,Atom[] l);
    }
}
