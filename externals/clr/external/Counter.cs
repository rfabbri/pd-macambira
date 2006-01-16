using System;

namespace PureData
{
	/// <summary>
	/// Descrizione di riepilogo per Counter.
	/// </summary>
	public class Counter
	{
		private IntPtr x;
		
		int curr = 0;

		public Counter()
		{
			curr = 0;
		}

		public Counter(float f)
		{
			curr = (int) f;
		}

		// this function MUST exist
		public void SetUp()
		{
			pd.AddSelector(x, "init", new pd.DelegateFloat(Init));
			pd.AddSelector(x, new pd.DelegateWithoutArguments(SendOut));
			pd.AddSelector(x, new pd.DelegateFloat(Sum));
			pd.AddInlet(x, "init", ParametersType.Float);
			pd.AddOutlet(x, ParametersType.Float);

		}

		public void Init(float f)
		{
			curr  = (int) f;
		}

		public void SendOut()
		{
			pd.SendToOutlet(x, 0, new Atom(curr));
		}

		public void Sum(float f)
		{
			curr += (int) f;
			pd.SendToOutlet(x, 0, new Atom(curr));
		}

	}
}
