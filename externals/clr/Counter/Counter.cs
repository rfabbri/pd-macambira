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

        AddInlet(s_list,new PureData.Symbol("list2"));
        AddInlet(ref farg);
        AddOutletBang();
    }

	// this function MUST exist
	// returns void or ClassType
	private static ClassType Setup(Counter obj)
	{
	    Add(new MethodBang(obj.MyBang));
        Add(new MethodFloat(obj.MyFloat));
        Add(new MethodSymbol(obj.MySymbol));
        Add(new MethodList(obj.MyList));
        Add("set",new MethodList(obj.MySet));
        Add("send",new MethodList(obj.MySend));
        Add(new MethodAnything(obj.MyAnything));

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

    protected virtual void MySet(PureData.AtomList l) 
    { 
        Post("Count-SET "+l.ToString()); 
        Outlet(0,new PureData.Symbol("set"),l);
    }

    protected virtual void MySend(PureData.AtomList l) 
    { 
        Send(new PureData.Symbol("receiver"),l);
        Send(new PureData.Symbol("receiver2"),(PureData.Atom[])l);
    }

    protected virtual void MyAnything(PureData.Symbol s,PureData.AtomList l) 
    { 
        Post("Count-("+s.ToString()+") "+l.ToString()); 
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
