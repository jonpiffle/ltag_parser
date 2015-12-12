import java.awt.MenuItem;

public class MyMenuItem extends MenuItem
{
    String leftChild, rightChild, parent;
    public MyMenuItem(String menuitemlabel){
	super(menuitemlabel);
    }
    public MyMenuItem(String nodeID, String leftchild, String rightchild, String menuitemlabel)
    {
	super(menuitemlabel);
	parent = nodeID;
	leftChild = leftchild;
	rightChild = rightchild;
	    
    }
    
}
