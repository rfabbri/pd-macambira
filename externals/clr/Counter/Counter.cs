using System;

/// <summary>
/// Descrizione di riepilogo per Counter.
/// </summary>
public class Counter:
	PureData.External
{
    PureData.Atom[] args;

    float farg;

    public Counter(PureData.AtomList args)
	{
        Post("Count.ctor "+args.ToString());

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
	private static ClassType Setup(Counter obj)
	{
	    AddMethod(0,new MethodBang(obj.MyBang));
        AddMethod(0,new MethodFloat(obj.MyFloat));
        AddMethod(0,new MethodSymbol(obj.MySymbol));
        AddMethod(0,new MethodList(obj.MyList));
        AddMethod(0,"set",new MethodAnything(obj.MySet));
        AddMethod(0,"send",new MethodAnything(obj.MySend));
        AddMethod(0,new MethodAnything(obj.MyAnything));
        AddMethod(1,new MethodFloat(obj.MyFloat1));
        AddMethod(1,new MethodAnything(obj.MyAny1));

        Post("Count.Main");
        return ClassType.Default;
	}

    protected virtual void MyBang() 
    { 
        Post("Count-BANG "+farg.ToString()); 
        Outlet(0);
    }

    protected virtual void MyFloat(float f) 
    { 
        Post("Count-FLOAT "+f.ToString()); 
        Outlet(0,f);
    }

    protected virtual void MyFloat1(float f) 
    { 
        Post("Count-FLOAT1 "+f.ToString()); 
    }

    protected virtual void MyAny1(int ix,PureData.Symbol s,PureData.AtomList l) 
    { 
        Post(ix.ToString()+": Count-ANY1 "+l.ToString()); 
    }

    protected virtual void MySymbol(PureData.Symbol s) 
    { 
        Post("Count-SYMBOL "+s.ToString()); 
        Outlet(0,s);
    }

    protected virtual void MyList(PureData.AtomList l) 
    { 
        Post("Count-LIST "+l.ToString()); 
        Outlet(0,l);
    }

    protected virtual void MySet(int ix,PureData.Symbol s,PureData.AtomList l) 
    { 
        Post("Count-SET "+l.ToString()); 
        Outlet(0,new PureData.Symbol("set"),l);
    }

    protected virtual void MySend(int ix,PureData.Symbol s,PureData.AtomList l) 
    { 
        Send(new PureData.Symbol("receiver"),l);
        Send(new PureData.Symbol("receiver2"),(PureData.Atom[])l);
    }

    protected virtual void MyAnything(int ix,PureData.Symbol s,PureData.AtomList l) 
    { 
        Post(ix.ToString()+": Count-("+s.ToString()+") "+l.ToString()); 
        Outlet(0,s,l);
    }
    /*
	public void SendOut()
	{
		pd.SendToOutlet(x, 0, new Atom(curr));
	}

	public void Sum(float f)
	{
		curr += (int) f;
		pd.SendToOutlet(x, 0, new Atom(curr));
	}

*/
}
