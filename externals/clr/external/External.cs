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
			pd.AddSelector(x, "selList", "SelList", ParametersType.List);
			pd.AddSelector(x, "selStringList", "SelStringList", ParametersType.List);
			pd.AddSelector(x, "selFloatList", "SelFloatList", ParametersType.List);
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
		}

		public void SelFloat(float f)
		{
			pd.PostMessage("SelFloat received " + f);
		}

		public void SelString(ref string s)
		{
			pd.PostMessage("SelString received " + s);
		}

		public void SelList(int [] list)
		{
			pd.PostMessage("SelList received " + list.Length + " elements");
			for (int i = 0; i<list.Length; i++)
			{
				pd.PostMessage("int " + i + " = " + list[i]);
			}
		}

		public void SelStringList(string [] list)
		{
			pd.PostMessage("SetStringList received a " + list.Length + " long list");
			for (int i = 0; i<list.Length; i++)
			{
				pd.PostMessage("string " + i + " = " + list[i]);
			}
		}

		public void SelFloatList(float [] list)
		{
			pd.PostMessage("SetStringList received a " + list.Length + " long list");
			for (int i = 0; i<list.Length; i++)
			{
				pd.PostMessage("float " + i + " = " + list[i]);
			}
		}

		public void SelGenericList(Atom [] list)
		{
			pd.PostMessage("SetStringList received a " + list.Length + " long list");
			for (int i = 0; i<list.Length; i++)
			{
				Atom a = (Atom) list[i];
				pd.PostMessage("list[" + i + "] is type " + a.type + " stringa = " + a.string_value);
			//	pd.PostMessage("float " + i + " = " + list[i]);
			}
		}

		public int test(int a)
		{
			

			Console.WriteLine("test("+a+")");
			return a+1;
		}
	}





}
