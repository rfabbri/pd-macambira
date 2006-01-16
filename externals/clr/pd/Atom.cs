using System;
using System.Runtime.InteropServices; // for structures

namespace PureData
{
	public enum AtomType {Null = 0, Float=1, Symbol=2, List=3, Bang=4};

	//[StructLayout (LayoutKind.Explicit)]
	[StructLayout (LayoutKind.Sequential)]
	public struct Atom 
	{
		public AtomType type;
		public float float_value;
		public string string_value;
		public Atom(float f)
		{
			this.type = AtomType.Float;
			this.float_value = f;
			this.string_value = "float";
		}
		public Atom(int i)
		{
			this.type = AtomType.Float;
			this.float_value = (float) i;
			this.string_value = "float";
		}
		public Atom(string s)
		{
			this.type = AtomType.Symbol;
			this.float_value = 0;
			this.string_value = s;
		}
	}
}