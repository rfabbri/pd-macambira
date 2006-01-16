using System;

namespace PureData
{
	
	public class External
	{
		private IntPtr x;
		
		public External()
		{
		}

		// this function MUST exist
		public void SetUp()
		{
			// now you can do what you like...
			Console.WriteLine("pointer set!");
			Console.WriteLine("setting selectors..");
			pd.AddSelector(x, "sel1", "Sel1", ParametersType.None);
			pd.AddSelector(x, "sel2", "Sel2", ParametersType.None);
			pd.AddSelector(x, "selFloat", "SelFloat", ParametersType.Float);
			pd.AddSelector(x, "selString", "SelString", ParametersType.Symbol);
			pd.AddSelector(x, "selGenericList", "SelGenericList", ParametersType.List);

			pd.AddSelector(x, "", "GetBang", ParametersType.Bang);
			pd.AddSelector(x, "", "GetFloat", ParametersType.Float);
			pd.AddSelector(x, "", "GetSymbol", ParametersType.Symbol);

			Console.WriteLine("selectors set");
			pd.AddOutlet(x, ParametersType.Float);
			pd.AddInlet(x, "selFloat", ParametersType.Float);
		}

		public void GetBang()
		{
			pd.PostMessage("GetBang invoked!");
		}

		public void GetFloat(float f)
		{
			pd.PostMessage("GetFloat invoked with " + f);
		}
		
		public void GetSymbol(ref string s)
		{
			pd.PostMessage("GetSymbol invoked with " + s);
		}
		
		public void Sel1()
		{
			pd.PostMessage("Sel1 invoked!");
			Atom [] a= new Atom[2];

		}

		public void Sel2()
		{
			pd.PostMessage("Sel2 invoked!");
			
			// testing outlets
			Atom[] atoms = new Atom[4];
			atoms[0] = new Atom(1.5f);
			atoms[1] = new Atom("ciao");
			atoms[2] = new Atom(2.5f);
			atoms[3] = new Atom("hello");
			pd.SendToOutlet(x, 0, atoms);

		}

		public void SelFloat(float f)
		{
			pd.PostMessage("SelFloat received " + f);


		}

		public void SelString(ref string s)
		{
			pd.PostMessage("SelString received " + s);
		}

		public void SelGenericList(Atom [] list)
		{
			Atom [] ret = new Atom[list.Length];
			for (int i = 0; i<list.Length; i++)
			{
				Atom a = (Atom) list[i];
				switch (a.type)
				{
					case (AtomType.Null):
					{
						pd.PostMessage("element null");
						break;
					}
					case (AtomType.Float):
					{
						pd.PostMessage("" + a.float_value);
						ret[i] = new Atom(a.float_value * 2);
						break;
					}
					case (AtomType.Symbol):
					{
						ret[i] = new Atom(a.string_value + "-lo-giuro");
						pd.PostMessage(a.string_value);
						break;
					}
				}
			}
			pd.SendToOutlet(x, 0, ret);
		}


	}





}
