using System;
using System.Runtime.CompilerServices;

namespace PureData
{
	public enum ParametersType {None = 0, Float=1, Symbol=2, List=3};

	public class pd
	{
		[MethodImplAttribute (MethodImplOptions.InternalCall)]
		private extern static void RegisterSelector (IntPtr x, string sel, string met, int type);
		// function called by the user
		public static void AddSelector(IntPtr x, string sel, string met, ParametersType type)
		{
			RegisterSelector (x, sel, met, (int) type);
		}

		// TODO
		// send stuff to an outlet
		[MethodImplAttribute (MethodImplOptions.InternalCall)]
		public extern static void ToOutlet (IntPtr x, int outlet, int type /*, ? array of values */);

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
