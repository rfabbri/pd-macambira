using System;

/// <summary>
/// Descrizione di riepilogo per Counter.
/// </summary>
public class Counter:
	PureData.External
{
    public Counter(PureData.AtomList args)
	{
        Post("Count.ctor "+args.ToString());

//        pd.AddInlet(x, "init", ParametersType.Float);
//        pd.AddOutlet(x, ParametersType.Float);
    }

	// this function MUST exist
	private static void Setup(Counter obj)
	{
	    Add(new MethodBang(obj.MyBang));
        Add(new MethodFloat(obj.MyFloat));
        Add(new MethodSymbol(obj.MySymbol));
        Add(new MethodList(obj.MyList));

        Post("Count.Main");
	}

    protected virtual void MyBang() 
    { 
        Post("Count-BANG"); 
    }

    protected virtual void MyFloat(float f) 
    { 
        Post("Count-FLOAT "+f.ToString()); 
    }

    protected virtual void MySymbol(PureData.Symbol s) 
    { 
        Post("Count-SYMBOL "+s.ToString()); 
    }

    protected virtual void MyList(PureData.AtomList l) 
    { 
        Post("Count-LIST "+l.ToString()); 
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
