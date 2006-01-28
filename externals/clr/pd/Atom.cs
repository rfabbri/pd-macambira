using System;
using System.Runtime.InteropServices; // for structures

namespace PureData
{
	public enum AtomType {Null = 0, Float = 1, Symbol = 2, Pointer = 3};

    [StructLayout (LayoutKind.Sequential)]
    sealed public class Symbol
    {
        // this should NOT be public
        readonly private IntPtr ptr;
        
        public Symbol(IntPtr p)
        {
            ptr = p;
        }

        public Symbol(Symbol s)
        {
            ptr = s.ptr;
        }

        public Symbol(string s)
        {
            ptr = Core.GenSym(s);
        }
        
        override public string ToString()
        {
            return Core.EvalSym(this);
        }
    }

    [StructLayout (LayoutKind.Sequential)]
    sealed public class Pointer
    {
        public IntPtr ptr;
    }

    [StructLayout (LayoutKind.Explicit)]
    public struct Word
    {
        [FieldOffset(0)] public float w_float;
        [FieldOffset(0)] public Symbol w_symbol;
        [FieldOffset(0)] public Pointer w_pointer;
    }

    //[StructLayout (LayoutKind.Explicit)]
	[StructLayout (LayoutKind.Sequential)]
	sealed public class Atom 
	{
	
		public AtomType type;
		public Word word;
		
		public Atom(float f)
		{
			type = AtomType.Float;
			word.w_float = f;
		}

		public Atom(int i)
		{
            type = AtomType.Float;
            word.w_float = (float)i;
        }

        public Atom(Symbol s)
        {
            type = AtomType.Symbol;
            word.w_symbol = s;
        }
        
		public Atom(string s)
		{
            type = AtomType.Symbol;
            word.w_symbol = new Symbol(s);
		}
	}
	
	
	// this struct is relative to this c struct, see clr.c

	/*
		// simplyfied atom
		typedef struct atom_simple atom_simple;
		typedef enum
		{
			A_S_NULL=0,
			A_S_FLOAT=1,
			A_S_SYMBOL=2,
		}  t_atomtype_simple;
		typedef struct atom_simple
		{
			t_atomtype_simple a_type;
			union{
				float float_value;
				MonoString *string_value;
			} stuff;
		};
		*/

}