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

        // --------------------------------------------------------------------------

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void AddInlet(void *obj,Symbol sel,Symbol to_sel);

        // map to data member
        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void AddInlet(void *obj,ref float f); 

        // map to data member
        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void AddInlet(void *obj,ref Symbol s);

        // map to data member
//        [MethodImplAttribute (MethodImplOptions.InternalCall)]
//        internal extern static void AddInlet(void *obj,ref Pointer f);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void AddInlet(void *obj,Symbol type); // create proxy inlet (typed)

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void AddInlet(void *obj); // create proxy inlet (anything)

        // --------------------------------------------------------------------------

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        internal extern static void AddOutlet(void *obj,Symbol type);

        // --------------------------------------------------------------------------

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

        // --------------------------------------------------------------------------

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

        protected readonly static Symbol _ = new Symbol("");
        protected readonly static Symbol _bang = new Symbol("bang");
        protected readonly static Symbol _float = new Symbol("float");
        protected readonly static Symbol _symbol = new Symbol("symbol");
        protected readonly static Symbol _pointer = new Symbol("pointer");
        protected readonly static Symbol _list = new Symbol("list");
        protected readonly static Symbol _anything = new Symbol("anything");

        // --------------------------------------------------------------------------

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Post(string message);        

        protected static void Post(string format,object arg0) { Post(String.Format(format,arg0)); }
        protected static void Post(string format,object arg0,object arg1) { Post(String.Format(format,arg0,arg1)); }
        protected static void Post(string format,object arg0,object arg1,object arg2) { Post(String.Format(format,arg0,arg1,arg2)); }
        protected static void Post(string format,params object[] args) { Post(String.Format(format,args)); }

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void PostError(string message);        

        protected static void PostError(string format,object arg0) { PostError(String.Format(format,arg0)); }
        protected static void PostError(string format,object arg0,object arg1) { PostError(String.Format(format,arg0,arg1)); }
        protected static void PostError(string format,object arg0,object arg1,object arg2) { PostError(String.Format(format,arg0,arg1,arg2)); }
        protected static void PostError(string format,params object[] args) { PostError(String.Format(format,args)); }

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void PostVerbose(int lvl,string message);        

        protected static void PostVerbose(int lvl,string format,object arg0) { PostVerbose(lvl,String.Format(format,arg0)); }
        protected static void PostVerbose(int lvl,string format,object arg0,object arg1) { PostVerbose(lvl,String.Format(format,arg0,arg1)); }
        protected static void PostVerbose(int lvl,string format,object arg0,object arg1,object arg2) { PostVerbose(lvl,String.Format(format,arg0,arg1,arg2)); }
        protected static void PostVerbose(int lvl,string format,params object[] args) { PostVerbose(lvl,String.Format(format,args)); }

        // --------------------------------------------------------------------------

        protected delegate void MethodBang();
        protected delegate void MethodFloat(float f);
        protected delegate void MethodSymbol(Symbol s);
        protected delegate void MethodPointer(Pointer p);
        protected delegate void MethodList(AtomList lst);
        protected delegate void MethodAnything(int inlet,Symbol tag,AtomList lst);

        // --------------------------------------------------------------------------

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void AddMethod(int inlet,MethodBang m);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void AddMethod(int inlet,MethodFloat m);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void AddMethod(int inlet,MethodSymbol m);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void AddMethod(int inlet,MethodPointer m);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void AddMethod(int inlet,MethodList m);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void AddMethod(int inlet,Symbol sel,MethodAnything m);

        protected static void AddMethod(int inlet,string sel,MethodAnything m) { AddMethod(inlet,new Symbol(sel),m); }

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void AddMethod(int inlet,MethodAnything m);

        // --------------------------------------------------------------------------

        protected void AddInlet(ref float f) { Internal.AddInlet(ptr,ref f); } // map to data member
        protected void AddInlet(ref Symbol s) { Internal.AddInlet(ptr,ref s); } // map to data member
//        protected void AddInlet(ref Pointer p) { Internal.AddInlet(ptr,ref p); } // map to data member
//        protected void AddInlet(Symbol type) { Internal.AddInlet(ptr,type); } // create typed inlet
        protected void AddInlet() { Internal.AddInlet(ptr); } // create inlet responding to any message
        protected void AddInlet(Symbol sel,Symbol to_sel) { Internal.AddInlet(ptr,sel,to_sel); } // redirect messages to defined selector

        // --------------------------------------------------------------------------

        protected void AddOutlet(Symbol type) { Internal.AddOutlet(ptr,type); }

        protected void AddOutletBang() { AddOutlet(_bang); }
        protected void AddOutletFloat() { AddOutlet(_float); }
        protected void AddOutletSymbol() { AddOutlet(_symbol); }
        protected void AddOutletPointer() { AddOutlet(_pointer); }
        protected void AddOutletList() { AddOutlet(_list); }
        protected void AddOutletAnything() { AddOutlet(_anything); }

        // --------------------------------------------------------------------------

        protected void Outlet(int n) { Internal.Outlet(ptr,n); }
        protected void Outlet(int n,float f) { Internal.Outlet(ptr,n,f); }
        protected void Outlet(int n,Symbol s) { Internal.Outlet(ptr,n,s); }
        protected void Outlet(int n,Pointer p) { Internal.Outlet(ptr,n,p); }
        protected void Outlet(int n,Atom a) { Internal.Outlet(ptr,n,a); }
        protected void Outlet(int n,AtomList l) { Internal.Outlet(ptr,n,_list,l); }
        protected void Outlet(int n,Atom[] l) { Internal.Outlet(ptr,n,_list,l); }
        protected void Outlet(int n,Symbol s,AtomList l) { Internal.Outlet(ptr,n,s,l); }
        protected void Outlet(int n,Symbol s,Atom[] l) { Internal.Outlet(ptr,n,s,l); }

        // --------------------------------------------------------------------------

        // bind to symbol
        protected void Bind(Symbol sym) { Internal.Bind(ptr,sym); }
        protected void Unbind(Symbol sym) { Internal.Unbind(ptr,sym); }

        // send to receiver symbol
        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Send(Symbol sym,Atom a);

        protected static void Send(Symbol sym) { Send(sym,_bang,null); }
        protected static void Send(Symbol sym,float f) { Send(sym,new Atom(f)); }
        protected static void Send(Symbol sym,Symbol s) { Send(sym,new Atom(s)); }
        protected static void Send(Symbol sym,Pointer p) { Send(sym,new Atom(p)); }
        protected static void Send(Symbol sym,AtomList l) { Send(sym,_list,l); }
        protected static void Send(Symbol sym,Atom[] l) { Send(sym,_list,l); }

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Send(Symbol sym,Symbol s,AtomList l);

        [MethodImplAttribute (MethodImplOptions.InternalCall)]
        protected extern static void Send(Symbol sym,Symbol s,Atom[] l);
    }
}
