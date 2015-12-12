import java.lang.*;
import java.util.*;


public class TreeNode
{
    // class variables 
    String nodeID, nodeLabel, nodeType;
    
    TreeNode ns; // nextSibling
    TreeNode ps; // previousSibling
    TreeNode fc; // first Child
    
    List features; // features of this node
    List collapsedNodeFeatures; // features of nodes that can be collapsed into this node
    List compressedNodes; // list of all nodes skipped during creation of derivedTree
    List collapsedNodes; // a list of nodes that can be collapsed into this node for a derived tree
    List menuItems; // menuitems for this node
    List collapsedNodeMenuItems; // menuitems for nodes that can be collapsed into this node
    
    String hNodeID; // node at which this node can be collapsed into
    boolean terminal; // flag to indicate whether this is the rightmost node amongst siblings
    boolean menuItemsListSet; // indicates whether popup menu items has been created for this node
    boolean collapseable; /// flag to indicate whether this node is collapseable
    boolean isCollapsed; // flag to indicate if this node is collapsed
    boolean processed;   // If this node is a root to a tree, this flag indicates whether the tree has been processed
    boolean auxfoot;
    boolean isInserted;
    TreeNode marker; // pointer to where drawTreeRecursive should jump to, from this node, if this node is collapsed
    TreeNode realChild;
    
    // constructors
    public TreeNode(){
	nodeID = null;
    }
    public TreeNode(String label, String NodeID)
    {
	nodeID=NodeID;
	nodeLabel = label;
	fc = ns = ps = null;
	terminal = true;
	compressedNodes = new List();
	menuItemsListSet = false;
	features=null;
	collapsedNodeFeatures=null;
	isCollapsed = true;
	collapsedNodes=new List();
	processed=false;
	realChild=null;
	auxfoot = false;
	isInserted=false;
    }
    // methods 
    public boolean isEmpty(){
	if(nodeID == null)
	    return true;
	else
	    return false;
    }
}





