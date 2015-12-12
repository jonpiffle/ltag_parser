import java.lang.*;
import java.util.*;


public class Tree
{
    TreeNode root;
    int leafs;  // number of leafs
    public Tree()
    {
	root = null ;
    }
    
    public Tree(String nodeLabel, String nodeID)
    {
	root = new TreeNode(nodeLabel, nodeID);
	root.terminal = false;
	leafs = 0;
    }
    
    // methods
    
    public void  printTree()
    {
	StringBuffer output = new StringBuffer();
	TreeNode currNode = root;
	if(currNode == null){
	    System.out.println("The root of this tree is empty");
	    System.exit(0);
	}
	while(currNode != null){
	    // if the current node is the node we are looking for return it immediately
	    output.append("(" + currNode.nodeLabel);
	    // if the first child is null, move up the tree or to the left if there are any siblings
	    if(currNode.fc == null){
		if(currNode.terminal){
		    // terminal nodes point back at their parent. Here we are going up the tree
		    currNode = currNode.ns;
		    while(currNode.terminal){
			output.append(")");
			currNode = currNode.ns;
		    }
		    output.append("))");
		    // you have reached the end of the tree
		    if(currNode.ns == null){
			break;
		    }
		    currNode = currNode.ns;
		}
		// if there are siblings to the current node, move to the next sibling
		else{
		    currNode = currNode.ns;
		    output.append(")");
		}
		
	    }

	    
	    // if the current node has children, then go down the tree and check these children
	    else{
		currNode = currNode.fc;
	    }

	}
	System.out.println(output.toString());
    }
    
    public TreeNode findNode(String nid)
    {
	TreeNode currNode = root;
	    // if the root is node we are looking for return it immediately
	    if(currNode.nodeID.compareTo(nid) == 0){
		return  currNode;
	    }
	    // if the root has no children and it is not the node we are looking for, then return immediately
	    if(currNode.fc == null){
		return null;
	    }
	    // while the first child of current node != null
	    currNode = currNode.fc;
	    while(currNode != null){
	        // if the current node is the node we are looking for return it immediately
	        if(currNode.nodeID.compareTo(nid) ==0){
		    return currNode;
		}
	        // if the first child is null, move up the tree or to the left if there are any siblings
	        if(currNode.fc == null) {
	    	    if(currNode.terminal){
			// terminal nodes point back at their parent. Here we are going up the tree
			currNode = currNode.ns;
    		        while(currNode.terminal){
		    	        // keep going up the tree
			    currNode = currNode.ns;
			}
    		        // you have reached the end of the tree
	    	        if(currNode.ns == null){
			    break;
			}
			currNode = currNode.ns;
		    }
	    	    // if there are siblings to the current node, move to the next sibling
		    else{
			currNode = currNode.ns;
		    }
	        }
	        // if the current node has children, then go down the tree and check these children
		else{
		    currNode = currNode.fc;
	        }
	    }
	    // return null
	    return null;
    }
    
    public int addChild(String pID, TreeNode child)
    {
	    TreeNode parent; // parent TreeNode of child
	    TreeNode currNode;
	    boolean end = false;
	    TreeNode terminalSibling;
	    // if the tree is empty, then set root to child
	    if(isEmpty())
	    {
	        root = child;
	        root.terminal = false;
	        return 1;
	    }
	    // find the parent
	    parent = findNode(pID);
	    // if parent wasn't found then return 0 immediately
	    if(parent == null)
	    {
	        return 0;
	    }
	    // if the parent has no children...
	    if(parent.fc == null)
		{
	        // the first child becomes the current terminal node
		    
		// if the previous node is null, then the child is a root node whose terminal=false by default.
		// change it to true immediately
		if(child.ps == null){
		    child.terminal=true;
		}
		
		// go the child's terminal sibling
		terminalSibling = child;
		while(terminalSibling != null){
		    if(terminalSibling.terminal){
			terminalSibling.ns = parent;
			break;
		    }
		    terminalSibling = terminalSibling.ns;
		}
		// the first child it points to the parent
	        child.ps = parent;
	        // set the first child of parent to this `child'
	        parent.fc = child;
		return 1;
	    }
	    // otherwise set currNode to the first child
	    currNode = parent.fc;
	    // go to the terminal node and insert the child there
	    while(!end)
	    {
	        if(currNode.terminal)
	        {
		    //System.out.println("Found Terminal Node");
		    end = true;
		    // make `child' terminal node
		    child.terminal = true;
		    currNode.terminal = false;
		    // go the terminal sibling of the child
		    if(child.ps == null){
			child.terminal=true;
		    }
		    
		    // go the child's terminal sibling
		    terminalSibling = child;
		    while(terminalSibling != null){
			if(terminalSibling.terminal){
			    terminalSibling.ns = parent;
			    break;
			}
			terminalSibling = terminalSibling.ns;
		    }
		    terminalSibling.ns = currNode.ns;
	    	    child.ps = currNode;
		    currNode.ns = child;
		    return 1;
	        }
		// move to the next sibling
	        currNode = currNode.ns;
	    }
	    return 0;
    }
    
    public int replaceNode(String nid, TreeNode replacement){
	TreeNode r; // node (subtree) being replaced
	TreeNode currNode;
	TreeNode previous;
	TreeNode lastChild, terminalSib;
	currNode = root;
	r = findNode(nid);
	// if the node was not found....
	if(r == null){
	    System.out.println("Node " + nid + " was not found");
	    return 0;
	}
	// if the node was found, get the previous sibling. 
	previous = r.ps;
	// this means that this node was the root of the tree
	if(previous == null){
	    lastChild = root.fc;
	    while(!lastChild.terminal){
		lastChild = lastChild.ns;
	    }
	    lastChild.ns = replacement;
	    replacement.ps = replacement.ns = null;
	    replacement.fc = root.fc;
	    root = replacement;
	    return 1;
	}
	// otherwise ....
	// if the replacement is the root of another tree, indicate that it is a terminal node
	if(replacement.ps == null){
	    replacement.terminal = true;
	}
	// go to the terminal sibling of replacement
	terminalSib = replacement;
	while(terminalSib != null){
	    if(terminalSib.terminal){
		// set the terminal flag to that of r
		terminalSib.terminal = r.terminal;
		// the replacement points to the next sibling previously pointed to by r
		terminalSib.ns = r.ns;
		break;
	    }
	    terminalSib= terminalSib.ns;
	}
	replacement.ps = previous;
	// if the node being replaced was the first child of previous, then set the first child of previous to replacement
	if(previous.fc.nodeID.compareTo(nid) ==0){
	    previous.fc = replacement;
	}
	else{
	    previous.ns = replacement;
	}
	return 1;
    }


    public static int findDepth(TreeNode tnode)
    {
	TreeNode currNode;
	int i = 0;
	int j = 0;
	boolean end;
	currNode= tnode;
	end = false;
	// if the node is a leaf, i.e. has no children
	if(currNode.fc == null){
	    //leafs++;
	    return 1;
	}
	// otherwise, get all the children of currNode and call find Depth;
	currNode = currNode.fc;
	while(!end){
	    if(currNode.terminal){
		end = true;
	    }
	    j = findDepth(currNode);
	    i = Math.max(i,j);
	    currNode = currNode.ns;
	}
	return ++i;
    }
    
    public boolean isEmpty(){
	if(root == null) 
	    return true;
	else
	    return false;
    }
    
	//{{DECLARE_CONTROLS
	//}}
    
    public static void main(String args[])
    {
	Tree t = new Tree("A","A");
	Tree tn = new Tree("B","B");
	System.out.println("Status: " +t.addChild("A", tn.root));
	tn = new Tree("C","C");
	System.out.println("Status: " +t.addChild("A", tn.root));
	t.printTree();
    }
}














