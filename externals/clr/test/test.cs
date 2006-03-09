using System;

public class test:
    PureData.External
{
    PureData.Atom[] args;

    float farg;

    public test(PureData.AtomList args)
    {
        Post("Test.ctor "+args.ToString());

        // that's the way to store args (don't just copy an AtomList instance!!)
        this.args = (PureData.Atom[])args;

        //        AddInlet(s_list,new PureData.Symbol("list2"));
        AddInlet();
        AddInlet(ref farg);
        AddInlet();
        AddOutletBang();
    }

    // this function MUST exist
    // returns void or ClassType
    private static ClassType Setup(test obj)
    {
        AddMethod(0,new Method(obj.MyBang));
        AddMethod(0,new MethodFloat(obj.MyFloat));
        AddMethod(0,new MethodSymbol(obj.MySymbol));
        AddMethod(0,new MethodList(obj.MyList));
        AddMethod(0,"set",new MethodAnything(obj.MySet));
        AddMethod(0,"send",new MethodAnything(obj.MySend));
        AddMethod(0,new MethodAnything(obj.MyAnything));
        AddMethod(1,new MethodFloat(obj.MyFloat1));
        AddMethod(1,new MethodAnything(obj.MyAny1));

        Post("Test.Main");
        return ClassType.Default;
    }

    protected virtual void MyBang() 
    { 
        Post("Test-BANG "+farg.ToString()); 
        Outlet(0);
    }

    protected virtual void MyFloat(float f) 
    { 
        Post("Test-FLOAT "+f.ToString()); 
        Outlet(0,f);
    }

    protected virtual void MyFloat1(float f) 
    { 
        Post("Test-FLOAT1 "+f.ToString()); 
    }

    protected virtual void MyAny1(int ix,PureData.Symbol s,PureData.AtomList l) 
    { 
        Post(ix.ToString()+": Test-ANY1 "+l.ToString()); 
    }

    protected virtual void MySymbol(PureData.Symbol s) 
    { 
        Post("Test-SYMBOL "+s.ToString()); 
        Outlet(0,s);
    }

    protected virtual void MyList(PureData.AtomList l) 
    { 
        Post("Test-LIST "+l.ToString()); 
        Outlet(0,l);
    }

    protected virtual void MySet(int ix,PureData.Symbol s,PureData.AtomList l) 
    { 
        Post("Test-SET "+l.ToString()); 
        Outlet(0,new PureData.Symbol("set"),l);
    }

    protected virtual void MySend(int ix,PureData.Symbol s,PureData.AtomList l) 
    { 
        Send(new PureData.Symbol("receiver"),l);
        Send(new PureData.Symbol("receiver2"),(PureData.Atom[])l);
    }

    protected virtual void MyAnything(int ix,PureData.Symbol s,PureData.AtomList l) 
    { 
        Post(ix.ToString()+": Test-("+s.ToString()+") "+l.ToString()); 
        Outlet(0,s,l);
    }
}
