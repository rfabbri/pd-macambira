package com.e1.pdj.test;

import junit.framework.TestCase;
import com.cycling74.max.Atom;

public class AtomTest extends TestCase {
    
	public void testIsIn() {
	    Atom[] list = new Atom[] { Atom.newAtom(1), Atom.newAtom("ok"), Atom.newAtom(5) }; 
	    
	    assertEquals(Atom.isIn(Atom.newAtom("ok"), list), 1);
	    assertEquals(Atom.isIn(Atom.newAtom(5), list), 2);
	    assertEquals(Atom.isIn(Atom.newAtom(0), list), -1);
	    assertEquals(Atom.isIn(Atom.newAtom(1), list, 1 ,2), -1);
	    assertEquals(Atom.isIn(Atom.newAtom("ok"), list, 1 ,2), 1);
	}

	public void testRemoveSome() {
	    Atom[] list = new Atom[] { Atom.newAtom(1), Atom.newAtom("ok"), Atom.newAtom(5) }; 

	    Atom[] test = Atom.removeSome(list, 1, 2);
	    assertEquals(1, test.length);
	    assertEquals(Atom.newAtom(1), test[0]);
	}

	public void testReverse() {
	    Atom[] list = new Atom[] { Atom.newAtom(1), Atom.newAtom("ok"), Atom.newAtom(5) };
	    
	    Atom[] test = Atom.reverse(list);
	    assertEquals(Atom.newAtom(5), test[0]);
	    assertEquals(Atom.newAtom("ok"), test[1]);
	    assertEquals(Atom.newAtom(1),test[2]);
	}
	
	public void testRotate() {
	    Atom[] list = new Atom[] { Atom.newAtom(1), Atom.newAtom("ok"), Atom.newAtom(5) };
	    
	    Atom[] test = Atom.rotate(list, 2);
	    assertEquals(Atom.newAtom("ok"), test[0]);
	    assertEquals(Atom.newAtom(5), test[1]);
	    assertEquals(Atom.newAtom(1), test[2]);
	    
	    test = Atom.rotate(list, 5);
	    assertEquals(Atom.newAtom("ok"), test[0]);
	    assertEquals(Atom.newAtom(5), test[1]);
	    assertEquals(Atom.newAtom(1), test[2]);
	    
	    test = Atom.rotate(list, 1);
	    assertEquals(Atom.newAtom(5), test[0]);
	    assertEquals(Atom.newAtom(1), test[1]);
	    assertEquals(Atom.newAtom("ok"), test[2]);
	}
	
	public void testRemoveFirst() {
	    Atom[] list = new Atom[] { Atom.newAtom(1), Atom.newAtom("ok"), Atom.newAtom(5) };
	    
	    Atom[] test = Atom.removeFirst(list, 2);
	    assertEquals(1, test.length);
	    assertEquals(Atom.newAtom(5), test[0]);

	    test = Atom.removeFirst(list);
	    assertEquals(2, test.length);
	    assertEquals(Atom.newAtom("ok"), test[0]);
	    assertEquals(Atom.newAtom(5), test[1]);
	}
	
	public void testRemoveLast() {
	    Atom[] list = new Atom[] { Atom.newAtom(1), Atom.newAtom("ok"), Atom.newAtom(5) };
	    
	    Atom[] test = Atom.removeLast(list, 2);
	    assertEquals(1, test.length);
	    assertEquals(Atom.newAtom(1), test[0]);
	    
	    test = Atom.removeLast(list);
	    assertEquals(2, test.length);
	    assertEquals(Atom.newAtom(1), test[0]);
	    assertEquals(Atom.newAtom("ok"), test[1]);
	}
	
	public void testUnion() {
	    assertTrue("union not implemented", false);
	}
	
	public void testIntersection() {
	    assertTrue("intersection not implementated", false);
	}
    
    public void testInt() {
        Atom test = Atom.newAtom(0x90);
        assertEquals(0x90, test.getInt());
    }
}
