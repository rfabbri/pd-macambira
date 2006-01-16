using System;
using System.Runtime.CompilerServices; // for extern import



namespace PureData
{
	public enum ParametersType {None = 0, Float=1, Symbol=2, List=3, Bang=4};

	public class pd
	{
		public delegate void DelegateWithoutArguments();
		public delegate void DelegateFloat(float f);
		public delegate void DelegateString(ref string s);
		public delegate void DelegateArray(Atom [] atoms);

		[MethodImplAttribute (MethodImplOptions.InternalCall)]
		private extern static void RegisterSelector (IntPtr x, string sel, string met, int type);
		// function called by the user
		public static void AddSelector(IntPtr x, string sel, DelegateWithoutArguments func)
		{
			RegisterSelector (x, sel, func.Method.Name, (int) ParametersType.None);
		}
		public static void AddSelector(IntPtr x, string sel, DelegateFloat func)
		{
			RegisterSelector (x, sel, func.Method.Name, (int) ParametersType.Float);
		}
		public static void AddSelector(IntPtr x, string sel, DelegateString func)
		{
			RegisterSelector (x, sel, func.Method.Name, (int) ParametersType.Symbol);
		}
		public static void AddSelector(IntPtr x, string sel, DelegateArray func)
		{
			RegisterSelector (x, sel, func.Method.Name, (int) ParametersType.List);
		}
		public static void AddSelector(IntPtr x, DelegateWithoutArguments func)
		{
			RegisterSelector (x, "", func.Method.Name, (int) ParametersType.None);
		}
		public static void AddSelector(IntPtr x, DelegateFloat func)
		{
			RegisterSelector (x, "", func.Method.Name, (int) ParametersType.Float);
		}
		public static void AddSelector(IntPtr x, DelegateString func)
		{
			RegisterSelector (x, "", func.Method.Name, (int) ParametersType.Symbol);
		}
		public static void AddSelector(IntPtr x, DelegateArray func)
		{
			RegisterSelector (x, "", func.Method.Name, (int) ParametersType.List);
		}

		// send stuff to an outlet
		[MethodImplAttribute (MethodImplOptions.InternalCall)]
		private extern static void ToOutlet (IntPtr x, int outlet, int atoms_length, Atom [] atoms);
		public static void SendToOutlet (IntPtr x, int outlet, Atom [] atoms)
		{
			ToOutlet (x, outlet, atoms.Length, atoms);
		}
		public static void SendToOutlet (IntPtr x, int outlet, Atom atom)
		{
			Atom [] atoms = new Atom[1];
			atoms[0] = atom;
			ToOutlet (x, outlet, atoms.Length, atoms);
		}
		public static void SendToOutlet (IntPtr x, int outlet, float f)
		{
			Atom [] atoms = new Atom[1];
			atoms[0] = new Atom(f);
			ToOutlet (x, outlet, atoms.Length, atoms);
		}
		public static void SendToOutlet (IntPtr x, int outlet, int i)
		{
			Atom [] atoms = new Atom[1];
			atoms[0] = new Atom((float) i);
			ToOutlet (x, outlet, atoms.Length, atoms);
		}
		public static void SendToOutlet (IntPtr x, int outlet, string s)
		{
			Atom [] atoms = new Atom[1];
			atoms[0] = new Atom(s);
			ToOutlet (x, outlet, atoms.Length, atoms);
		}

		// create an outlet
		[MethodImplAttribute (MethodImplOptions.InternalCall)]
		private extern static void CreateOutlet (IntPtr x, int type);
		// function called by the user
		public static void AddOutlet(IntPtr x, ParametersType type)
		{
			CreateOutlet (x, (int) type);
		}

		// create an inlet
		[MethodImplAttribute (MethodImplOptions.InternalCall)]
		private extern static void CreateInlet (IntPtr x, string selector, int type);
		// function called by the user
		public static void AddInlet(IntPtr x, string selector, ParametersType type)
		{
			CreateInlet (x, selector, (int) type);
		}

		[MethodImplAttribute (MethodImplOptions.InternalCall)]
		public extern static void PostMessage (string message);

		[MethodImplAttribute (MethodImplOptions.InternalCall)]
		public extern static void ErrorMessage (string message);

	}


	

}
