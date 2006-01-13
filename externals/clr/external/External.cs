using System;


namespace PureData
{
	


	public class External
	{
		private IntPtr x;
		
		public External()
		{
			x = IntPtr.Zero;
		}

		

		// this function MUST exist
		public void SetUp(IntPtr pdClass)
		{
			// you must assign pdclass to x !
			x = pdClass;

			// now you can do what you like...
			Console.WriteLine("pointer set!");
			Console.WriteLine("setting selectors..");
			pd.AddSelector(x, "sel1", "Sel1", ParametersType.None);
			pd.AddSelector(x, "sel2", "Sel2", ParametersType.None);
			pd.AddSelector(x, "selFloat", "SelFloat", ParametersType.Float);
			pd.AddSelector(x, "selString", "SelString", ParametersType.Symbol);
			pd.AddSelector(x, "selGenericList", "SelGenericList", ParametersType.List);
			Console.WriteLine("selectors set");
			pd.AddOutlet(x, ParametersType.Float);
			pd.AddInlet(x, "selFloat", ParametersType.Float);
		}



		public void Sel1()
		{
			pd.PostMessage("Sel1 invoked!");
		}

		public void Sel2()
		{
			pd.PostMessage("Sel2 invoked!");
			
			// testing outlets
			Atom[] atoms = new Atom[2];
			atoms[0] = new Atom("ciao");
			atoms[1] = new Atom(1.5f);
			pd.ToOutlet(x, 0, atoms.Length, atoms);

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
						break;
					}
					case (AtomType.Symbol):
					{
						pd.PostMessage(a.string_value);
						break;
					}
				}
			}		
		}


	}





}
