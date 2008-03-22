import com.cycling74.max.*;

public class help_class extends MaxObject {
	int attr1;
	
	public help_class() {
		declareOutlets(new int[] { DataTypes.FLOAT, DataTypes.ALL });
	}
	
	public void inlet(float x) {
		outlet(0, x);
	}
	
	public void bang() {
		outlet(1, "BANG! received");
	}
	
	public void callme(Atom args[]) {
		outlet(1, "callme has called with arg1:" + args[0].toString());
	}

	public void dynamic_method() {
		outlet(1, "dynamic_method has been called");
	}

}
