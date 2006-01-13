using System;
using System.Runtime.CompilerServices; // for extern import
using System.Runtime.InteropServices; // for structures


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
		public extern static void ToOutlet (IntPtr x, int outlet, int atoms_length, [In, Out] Atom [] atoms);
		public static void SendToOutlet (IntPtr x, int outlet, [In, Out] Atom [] atoms)
		{
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
	public enum AtomType {Null = 0, Float=1, Symbol=2, List=3};

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
		public Atom(string s)
		{
			this.type = AtomType.Symbol;
			this.float_value = 0;
			this.string_value = s;
		}
	}

		/*
			typedef float t_floatarg;  

		typedef struct _symbol
		{
			char *s_name;
			struct _class **s_thing;
				struct _symbol *s_next;
				} t_symbol;

				EXTERN_STRUCT _array;
		#define t_array struct _array      



		#define GP_NONE 0       
		#define GP_GLIST 1      
		#define GP_ARRAY 2      

		typedef struct _gstub
				{
					union
					{
					struct _glist *gs_glist;    
						struct _array *gs_array;    
						} gs_un;
						int gs_which;                   
						int gs_refcount;                
					} t_gstub;

		typedef struct _gpointer         
				{
					union
					{   
					struct _scalar *gp_scalar;  
						union word *gp_w;         
					} gp_un;
					int gp_valid;                  
					t_gstub *gp_stub;               
				} t_gpointer;

		typedef union word
						{
		t_float w_float;
		t_symbol *w_symbol;
		t_gpointer *w_gpointer;
		t_array *w_array;
		struct _glist *w_list;
			int w_index;
		} t_word;

		typedef enum
			{
				A_NULL,
				A_FLOAT,
				A_SYMBOL,
				A_POINTER,
				A_SEMI,
				A_COMMA,
				A_DEFFLOAT,
				A_DEFSYM,
				A_DOLLAR, 
				A_DOLLSYM,
				A_GIMME,
				A_CANT
			}  t_atomtype;

		#define A_DEFSYMBOL A_DEFSYM    

		typedef struct _atom
				{
					t_atomtype a_type;
					union word a_w;
				} t_atom;
		*/

}
