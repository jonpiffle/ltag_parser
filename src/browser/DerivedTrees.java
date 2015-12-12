import java.io.*; 
import java.util.*;
import java.lang.*;

public class DerivedTrees{
    
    // declare hashtables
    static  Hashtable graph;
    static  Hashtable memo;
    static  Hashtable altered;
    
    // other parameters
    String start, epsilon, top, bottom, insertLabel, empty_dtr, nullfoot, XTAG_INSERTED_FLAG,TREE_BANK_INSERTED_FLAG, SUBSCRIPT_MARKER;
    static String leaf;
    static boolean unification=false;
    boolean artificialNodesPresent=false;
    boolean XTAG_TREES, TREEBANK_TREES;
    String alteredNode="";
    String chosenDtrPair = "";
    int  startNumber;
    
    int HASHSIZE;
    int SMALLHASHSIZE;
    
    graphNode gNode;
    ShowTrees currentShowTrees;
    public DerivedTrees(ShowTrees st)
    {
	currentShowTrees= st;
	HASHSIZE=5000;
	SMALLHASHSIZE = 50;
	graph  =    new Hashtable(HASHSIZE);
	memo =      new Hashtable(HASHSIZE);
	altered   = new Hashtable(SMALLHASHSIZE);
	
	startNumber=0;
	empty_dtr = "(nil)";
	leaf      = "__leaf__";
	start     = "start";
	epsilon   = "e";
	top       = "top";
	bottom    = "bot";
	insertLabel= "__INSERT_SUBTREE_HERE__";
	nullfoot = "";
	
	XTAG_INSERTED_FLAG = "@";
	TREE_BANK_INSERTED_FLAG = "ins";
    }

    public String getWordFromTree(String treeName)
    {
	String word="";
	word=treeName.substring(treeName.indexOf("[")+1, treeName.indexOf("]"));
	if(word.compareTo("nil")==0){    return epsilon; }
	return word;
    }
 
    public List computeDerived()
    {
	int i=0;
	String nodeID;
	Tree subTree;
	List startNodes = new List();
	List treeList = new List();
	// create a List with a start node from which `all start nodes' can be selected
	List Start = new List();
	List resultList;
	Tree temp;
	String treeName;
	graphNode gNode;
	graph.put(empty_dtr, new graphNode(empty_dtr, "", "(nil)", "" , "", "", "", "EMPTYDTR", null, false));
	if(startNumber==0){
	    //System.out.println("The graph loaded is empty or has no valid start Nodes");
	    (new Error(currentShowTrees, 1, "The graph loaded is empty or has no valid start Nodes")).showErrMsg();
	    return null;
	}
	else{
	    for(i = 0; i < startNumber; i++){
		nodeID=start + "_" + i;
		// get the first actual node. Start nodes are artificial and only point to the beginning of the tree
		gNode = (graphNode)graph.get(nodeID);
		// lookup the treename of the first actual name
		treeName=((graphNode)graph.get(gNode.daughters)).treeName;
		resultList = computeDerivedRecursive(gNode.daughters, "0");
		subTree = (Tree)resultList.head.next.data;
		temp = new Tree("Start", "Start");
		temp.addChild("Start", subTree.root);
		temp.root.nodeType = "s";
		treeList.put(temp);
		startNodes.put(nodeID + ": " + treeName);
	    }
	    Start.put(treeList);
	    Start.put(startNodes);
	}
	return(Start);
    }
    
    public List computeDerivedRecursive(String nodeID, String mem)
    {
	graphNode gNode;
	Tree result= new Tree();
	String newInsertLabel;
	TreeNode newNode;
	StringTokenizer dtrOrList;
	int i=0;
	int j=0;
	int l=0; 
	String dtrPair;
	String left, right="";
	int count = 0;
	List featList=null;
	ListNode fl;
	List resultList = new List();
	// get the node and all its information
	gNode = (graphNode)graph.get(nodeID);
	// make sure that you have a valid node ID
	if(gNode.nodeType == null){
	    System.out.println("Node \""+ nodeID + "\" : Exiting because nodeType==NULL");
	    System.exit(-1);
	}
	// if the node is an empty tree .......
	if(gNode.nodeType.compareTo("EMPTYDTR") == 0){
	    result = new Tree("EMPTYDTR", nodeID);
	    result.root.nodeType = "e";
	    resultList.put(result);
	    return (resultList);
	}
	// if you have memorized the node ....
	if(memo.containsKey(nodeID)){
	    // get the subtree anchored by this nodeID
	    result = (Tree)memo.get(nodeID);
	    // check if the subtree contains an "insert_here" label and if it does replace the label with the new label
	    newNode = new TreeNode(mem + ":" + insertLabel, insertLabel);
	    // all the positions where an AUXFOOT exists, the nodes have nodeID insertLabel
	    result.replaceNode(insertLabel, newNode);
	    resultList.put(result);
	    return(resultList);
	}
	// if the node is a start node, then call this routine with its child. i.e. this is the starting point
	if(nodeID.startsWith(start)){
	    return computeDerivedRecursive(gNode.daughters, mem);
	}
	if(!graph.containsKey(nodeID)){
	    System.out.println("Error: \"" + nodeID +  "\" does not exist in graph. Exiting ...");  
	    System.exit(-1);
	}
	// if this node is the foot of an auxilliary node, mark it as an insertion point
	if(gNode.nodeType.compareTo("auxfoot")==0){
	    // insertLabel is the nodeID for all nodes that are replaced by AUXFOOT nodes
	    result= new Tree(mem + ":" + insertLabel, insertLabel);
	    result.root.nodeType = "af";
	    resultList.put(result);
	    return resultList;
	}
	try{
	    // extract the individual daughter pairs
	    dtrOrList= new StringTokenizer(gNode.daughters, "]");
	    // count the number of daughters pairs
	    count=dtrOrList.countTokens();
	    // All Nodes have Daughters: leafs have <LEAF> as a daughter which is artificially added
	    if(count==0){
		System.out.println("No Daughters found for Node: " + nodeID);
		System.exit(-1);
	    }
	    // check to see if the daughter pair is a LEAF
	    dtrPair=(dtrOrList.nextToken()).substring(1);
	    // Leaf Node: Get  features associated with it
	    if((dtrPair.compareTo(leaf)==0) && (gNode.nodeType.compareTo("auxfoot") != 0)){
		// create a separate node to indicate that this node is a bottom node
		result = new Tree(gNode.nodeLabel+ ".b", nodeID);
		result.root.nodeType="b";
		// create a leaf node
		Tree leaf= new Tree(getWordFromTree(gNode.treeName), nodeID);
		leaf.root.nodeType = "l";
		// add the leaf node to the current node
		result.addChild(nodeID, leaf.root);
		//get the features
		if(unification){
		    if(gNode.features !=null){
			fl=gNode.features.head;
			featList=new List();
			while(fl.next !=null){
			    featList.put(fl.next.data);
			    fl=fl.next;
			}
		    }
		}
		// add the features and the result to the result list
		resultList.put(result);
		resultList.put(featList);
		return (resultList);
	    }
	    // Altered Nodes: Selected by User using Popup Menus in the tree
	    if(altered.containsKey(nodeID)){
	        // if the user has selected a daughter pair for this node, get that daughter pair from hashtable
	        chosenDtrPair = (String)altered.get(nodeID);
	        left=chosenDtrPair.substring(0,l=dtrPair.indexOf(","));
		right=chosenDtrPair.substring(l+1);
	    }
	    else{
		left=dtrPair.substring(0,l=dtrPair.indexOf(","));
		right=dtrPair.substring(l+1);
	    }
	    resultList=getAndPairResults(nodeID, left, right, mem);
	    return resultList;
	}
	catch(Exception e){
	    System.out.println("Problematic Daughter list: " + gNode.daughters);
	    e.printStackTrace();
	}
	memo.put(nodeID, result);
	return resultList;
    }
    
    
    public List getAndPairResults(String nodeID, String left, String right, String mem)
    {
	String nodeLabel, rootLabel, nodePosition;
	String leftType, rightType;
	Tree result = new Tree();
	Tree leftResult, rightResult;
	boolean isNodeRoot, isInserted;
	boolean isNodeAuxFoot=false;
	List resultList = new List();
        List combinedFeatures;
	List leftResultList, rightResultList, auxResultList;
        List leftFeatures = null;
	List rightFeatures=null;
	
	graphNode gNode = (graphNode)graph.get(nodeID);
	graphNode leftNode = (graphNode)graph.get(left);
	graphNode rightNode = (graphNode)graph.get(right);
	
	leftType = leftNode.nodeType;
	rightType =rightNode.nodeType;
	
	nodeLabel = gNode.nodeLabel;
	rootLabel = gNode.rootLabel;
	nodePosition = gNode.position;
	isInserted=gNode.isInserted;
	// if both are Aux Roots, we have an error. Quit
	if((leftType.compareTo("auxroot") == 0) && (rightType.compareTo("auxroot")==0)){
	    System.out.println("Error: both left and right cannot be Aux roots");
	    System.exit(-1);
	}
	// get the results from the computing the derived tree on both the left and right daughters
	leftResultList  = computeDerivedRecursive(left, ((leftType.compareTo("auxroot") == 0) ? nodeID : mem));
	rightResultList = computeDerivedRecursive(right,((rightType.compareTo("auxroot")==0) ? nodeID : mem));
	
	if(nodeLabel.compareTo(rootLabel)==0 && nodePosition.compareTo("top")==0){     isNodeRoot=true; }
	else{  isNodeRoot=false; }
	
	// check to see if there are features from the left subtree 
	if(leftResultList.head.next.next != null){     leftFeatures = (List)leftResultList.head.next.next.data; }
	// check to see if there are features from the right subtree
	if(rightResultList.head.next.next != null){    rightFeatures = (List)rightResultList.head.next.next.data; }
	// combine the features
	combinedFeatures = mergeFeatures(leftFeatures, rightFeatures);
	// get the left and right subtrees
	leftResult = (Tree)leftResultList.head.next.data;
	rightResult= (Tree)rightResultList.head.next.data;
	// check if the current root node is an Auxilliary Foot
	if(leftResult.root.nodeType.compareTo("af")==0 || rightResult.root.nodeType.compareTo("af")==0 ){
	    isNodeAuxFoot=true;
	}
	// Substitution   Note: In substitution, Either LeftResult or rightResult is node empty.
	if(leftType.compareTo("initroot")==0){
	    if(nodePosition.compareTo(top)==0){ result = new Tree(gNode.nodeLabel+ ".t", nodeID); }
	    else{  result = new Tree(gNode.nodeLabel + ".b", nodeID); }
	    result.root.nodeType = "+";
	    result.addChild(nodeID, leftResult.root);
	}
	else if(rightType.compareTo("initroot")==0){
	    if(nodePosition.compareTo(top)==0){  result = new Tree(gNode.nodeLabel+ ".t", nodeID); }
	    else{   result = new Tree(gNode.nodeLabel+ ".b", nodeID); }
	    result.root.nodeType = "+";
	    result.addChild(nodeID, rightResult.root);
	}
	// Adjunction: if the left node is the auxilliary root node, then 
	else if(leftType.compareTo("auxroot") == 0){
	    if(nodePosition.compareTo(top)==0){   result = new Tree(gNode.nodeLabel + ".t", nodeID); }
	    else{   result = new Tree(gNode.nodeLabel + ".b", nodeID); }
	    // indicate that there is a real child that has been relocated by adjunction
	    result.root.realChild = rightResult.root;
	    result.root.nodeType = "*";
	    leftResult.replaceNode(insertLabel, rightResult.root);
	    result.addChild(nodeID,leftResult.root);
	}
	// otherwise, if the right node is the auxilliary root,
	else if(rightType.compareTo("auxroot") == 0){
	    if(nodePosition.compareTo(top)==0){  result = new Tree(nodeLabel+ ".t", nodeID); }
	    else{  result = new Tree(nodeLabel+ ".b", nodeID); }
	    result.root.realChild = leftResult.root;
	    result.root.nodeType = "*";
	    rightResult.replaceNode(insertLabel, leftResult.root);
	    result.addChild(nodeID,rightResult.root);
	}
	// completion: internal Branch 
	else if(gNode.position.compareTo(bottom)==0)
	    {
		result = new Tree(gNode.nodeLabel+ ".b", nodeID);
		result.root.nodeType = "b";
		result.root.isInserted=isInserted;
		if(leftType.compareTo("EMPTYDTR")==0){  result.addChild(nodeID, rightResult.root); }
		else if(rightType.compareTo("EMPTYDTR") == 0){  result.addChild(nodeID, leftResult.root); }
		else{
		    result.addChild(nodeID, leftResult.root);
		    result.addChild(nodeID, rightResult.root);
		}
	    }
	//*********** completion: internal top ************
	else if(gNode.position.compareTo(top)==0){
	    result = new Tree(gNode.nodeLabel+ ".t", nodeID);
	    result.root.nodeType = "t";
	    result.root.isInserted=isInserted;
	    if(leftType.compareTo("EMPTYDTR") == 0){  result.addChild(nodeID, rightResult.root); }
	    else if(rightType.compareTo("EMPTYDTR") == 0){   result.addChild(nodeID, leftResult.root); }
	    else{
		System.out.println("Node has 2 non-empty daughters");
		System.exit(-1);
	    }
	}
	else{   
	    System.out.println("Should not reach here"); 
	    System.out.println("Node Type: " + gNode.nodeType + "\tNode ID: " + gNode.nodeID + "\t Node Position: " + gNode.position);
	}
	// if the current node is a root node (internal top or auxroot), make sure that you set the features, otherwise just pass up the features
	resultList.put(result);
	result.root.auxfoot=isNodeAuxFoot;
	if(isNodeRoot){  
	    result.root.features=combinedFeatures; 
	    

	}
	else {    resultList.put(combinedFeatures); }
	return resultList;
    }
    
    public boolean isInserted(String n){
	if(XTAG_TREES){
	    if(n.compareTo(XTAG_INSERTED_FLAG)==0){	return(true); }
	}
	else if(TREEBANK_TREES){
	    // get the subscript
	    if(n.indexOf(SUBSCRIPT_MARKER) != -1){
		String subscript = n.substring(n.indexOf(SUBSCRIPT_MARKER)+1);
		if(subscript.compareTo(TREE_BANK_INSERTED_FLAG)==0){ return true; }
	    }
	}
	//else{     System.out.println("Error: treetype=" + TREETYPE + " not implemented in is_inserted"); }
	return(false);
    }
    
    public List  mergeFeatures(List leftFeat, List rightFeat){
        List result=null;
	ListNode l,r;
	String lf,rf;
	boolean repeated=false;
	// if both the left and the right features are not null, merge them and eliminate duplicates
	if(leftFeat != null && rightFeat != null){
	    r=rightFeat.head;
	    // for each feature in the right subtree, check to see if it is duplicated in the left subtree features
	    while(r.next !=null){
		// get the next feature from the right subtree
		rf=(String)r.next.data;
		l=leftFeat.head;
		// check each of the features in the left subtree to see if there is a duplicate
		while(l.next !=null){
		    lf=(String)l.next.data;
		    if(lf.compareTo(rf)==0){
			repeated=true;
			break;
		    }
		    else{
			l=l.next;
		    }
		}
		if(!repeated){ // add the feature to leftFeat
		    leftFeat.put(rf);
		}
		else{ // do not duplicate the feature if it already exists in leftFeat. Reset the flag.
		    repeated = false;
		}
		r=r.next;
	    }
	    result= leftFeat;
	}
	else if(leftFeat != null && rightFeat == null){    result =leftFeat;  }
	else if(rightFeat != null && leftFeat==null){   result =rightFeat; }
	return result;
    }
    
    
    public List computeDerivation()
    {
	int i =0;
	List all = new List();
	// a list of all the start nodes that are present in the graph
	List startNodes = new List();
	// a list of all the subtrees anchored by each start node
	List treeList = new List();
	// subtree anchored by each start node
	Tree subtree;
	String nodeID;
	Tree temp;
	String parentNodeLabel ="start"; // the parent node for the root node is null
	graphNode gNode;
	String treeName;
	if(!graph.containsKey(empty_dtr)){
	    graph.put(empty_dtr, new graphNode(empty_dtr, "", "", "" , "", "", "", "EMPTYDTR", null, false));
	}
	if(startNumber==0){
	    (new Error(currentShowTrees, 1, "The graph loaded is empty or has no valid start Nodes")).showErrMsg();
  	    return null;
	}
	for(i=0; i < startNumber; i++){
	    nodeID=start + "_" + i;
	    // get the first actual node. Start nodes are artificial and only point to the beginning of the tree
	    gNode = (graphNode)graph.get(nodeID);
	    // lookup the treename of the first actual name
	    treeName=((graphNode)graph.get(gNode.daughters)).treeName;
	    subtree = (Tree)computeDerivationRecursive(gNode, parentNodeLabel).head.next.data;
	    temp = new Tree("Start", "Start");
	    temp.addChild("Start", subtree.root);
	    temp.root.nodeType = "s";
	    // add the subtree to the list of subtrees
	    treeList.put(temp);
	    // add the nodeID for the start node
	    startNodes.put(nodeID + ": " + treeName);
	}
	all.put(treeList);
	all.put(startNodes);
	return(all);
    }
    
    public List computeDerivationRecursive(graphNode gNode, String parentNodeLabel)
    {
	graphNode leftNode, rightNode;
	String left, right;
	String nodeType="";
	String daughters="";
	StringTokenizer dtrOrList;
	int count, comma, i=0;
	String nodeLabel, token;
	Tree result = new Tree();
	Tree rightResultTree, leftResultTree;
	List rightResult, leftResult;
	TreeNode treenode, nextTreeNode;
	List dtrList = new List();
	// resultList contains a derivation tree and a list of other possible daughter pairs at this node
	List resultList = new List();
	List leftDtrList, rightDtrList;
	if(gNode.nodeType == null){
	    System.out.println("Node type for  " + gNode.nodeID + " not found. Exiting ....");
	    System.exit(-1);
	}
	if(gNode.nodeType.compareTo("EMPTYDTR") == 0){
	    // the tree is the first element, then the dtrOrList in the list
	    resultList.put(result);
	    resultList.put(dtrList);
	    return(resultList);
	}
	if(memo.containsKey(gNode.nodeID)){
	    result = (Tree)memo.get(gNode.nodeID);
	    resultList.put(result);
	    resultList.put(dtrList);
	    return(resultList);
	}
	if(!graph.containsKey(gNode.nodeID)){
	    System.out.println("Node " + gNode.nodeID + " not found in graph. Exiting....");
	    System.exit(-1);
	}
	if(gNode.nodeID.startsWith(start)){
	    return(computeDerivationRecursive((graphNode)graph.get(gNode.daughters), parentNodeLabel));
	}
	dtrOrList=new StringTokenizer(gNode.daughters, "]");
	count= dtrOrList.countTokens();
	if(gNode.daughters == null){
	    System.out.println("Error: node " + gNode.nodeID + " has no valid daughters. Exiting...");
	    System.exit(0);
	}
	// if the node is a root node, put it in the tree
	if((gNode.position.compareTo(top)==0) && 
	   ((gNode.nodeType.compareTo("auxroot")==0) || (gNode.nodeType.compareTo("initroot")==0))){
	    if(parentNodeLabel.compareTo("start")==0){
		nodeLabel=gNode.treeName;
	    }
	    else{
		nodeLabel=gNode.treeName + "<" + parentNodeLabel + ">";
	    }
	}					 
	else{ 
	    nodeLabel=null;
	}
	// get the first daughter pair
	token=dtrOrList.nextToken();
	// if the pair is a leaf, then return an empty tree
	if((token.substring(1)).compareTo(leaf)==0){
	    resultList.put(result);
	    resultList.put(dtrList);
	    return(resultList);
	}
	// if the node has been altered by the user, get the daughter pair selected by the user
	if(altered.containsKey(gNode.nodeID)){
	    token = (String)altered.get(gNode.nodeID);
	    left= token.substring(0,comma=token.indexOf(","));
	    right=token.substring(comma+1);
	}
	// otherwise extract the left and right from the first daugther pair
	else{
	    comma=token.indexOf(",");
	    left = token.substring(1,comma);
	    right = token.substring(comma+1);
	}
	leftNode = (graphNode)graph.get(left);
	rightNode =(graphNode)graph.get(right);
	// compute the left and right derivation trees
	leftResult = computeDerivationRecursive(leftNode, gNode.nodeLabel);
	rightResult = computeDerivationRecursive(rightNode, gNode.nodeLabel);
	// get the left and right subtrees
	leftResultTree= (Tree)leftResult.head.next.data;
	rightResultTree=(Tree)rightResult.head.next.data;
	// get the daughter lists for the left and right children
	leftDtrList = (List)leftResult.head.next.next.data;
	rightDtrList = (List)rightResult.head.next.next.data;
	
	// if both left and right subtrees are empty...
	if(leftResultTree.isEmpty() && rightResultTree.isEmpty()){
	    // if the current gNode is a root node, i.e. its nodeLabel has not been set to null
	    if(nodeLabel != null){
		// create a tree with this root node
		result = new Tree(nodeLabel, gNode.nodeID);
		// if the left and right nodes had multiple children, you must remember them at this node
		dtrList.concatenate(leftDtrList);
		dtrList.concatenate(rightDtrList);
		result.root.compressedNodes = dtrList;
		// make the list empty since all the dtrs are accounted for in this current node 
		dtrList = new List();
	    }
	    else{
		// if this node is null, concatenate the left and right with the new list
		dtrList.put(gNode.nodeID);
		dtrList.concatenate(leftDtrList);
		dtrList.concatenate(rightDtrList);
	    }
	} 
	else if(leftResultTree.isEmpty()){
	    if(nodeLabel != null){
		result = new Tree(nodeLabel, gNode.nodeID);
		// add the right result to this tree
		// add the right result to the new tree just created. 
		// The type of tree returned here is special because it may consist of a few subtrees that are linked by the roots. This case
		// occurs when a node that is not a root node has none-empty children that need to be passed up to the closest root node above. In such
		// a case, the first child's root node has a pointer (child1.root.ns=child2.root) to the second child's root. Hence you must remember to add
		// all the children to the result
		treenode = rightResultTree.root;
		while(treenode != null){
		    nextTreeNode = treenode.ns;
		    result.addChild(gNode.nodeID, treenode);
		    treenode = nextTreeNode;
		}
		dtrList.concatenate(leftDtrList);
		dtrList.concatenate(rightDtrList);
		result.root.compressedNodes = dtrList;
		// make the list empty since all the dtrs are accounted for in this current node
		dtrList = new List();
	    }
	    else{
		result = rightResultTree;
		// concatenate the dtrList with the left and right dtr lists
		dtrList.put(gNode.nodeID);
		dtrList.concatenate(leftDtrList);
		dtrList.concatenate(rightDtrList);
	    }
	}
	// if the right subtree is empty, ..
	else if(rightResultTree.isEmpty()){
	    // if this current gNode is a root node
	    if(nodeLabel != null){
		// create a new tree with this gNode
		result = new Tree(nodeLabel, gNode.nodeID);
		// add the right result to the new tree just created. 
		// The type of tree returned here is special because it may consist of a few subtrees that are linked by the roots. This case
		// occurs when a node that is not a root node has none-empty children that need to be passed up to the closest root node above. In such
		// a case, the first child's root node has a pointer (child1.root.ns=child2.root) to the second child's root. Hence you must remember to add
		// all the children to the result
		treenode = leftResultTree.root;
		while(treenode != null){
		    nextTreeNode = treenode.ns;
		    result.addChild(gNode.nodeID, treenode);
		    treenode = nextTreeNode;
		}
		// remember the left and right children's daughter pairs
		dtrList.concatenate(leftDtrList);
		dtrList.concatenate(rightDtrList);
		result.root.compressedNodes = dtrList;
		// make the list empty since all the dtrs are accounted for in this current node
		dtrList = new List();
	    }
	    // if the current node is not a root node, 
	    else{
		// set the result to the left result
		result = leftResultTree;
		// set the daughter list to include the left, right and current nodes daughter lists
		dtrList.put(gNode.nodeID);
		dtrList.concatenate(leftDtrList);
		dtrList.concatenate(rightDtrList);
	    }
	}
	// if both subtrees are none-empty
	else{
	    // if this node is a root node, ......
	    if(nodeLabel != null){
		result = new Tree(nodeLabel, gNode.nodeID);
		// add the children starting with the leftmost
		treenode = leftResultTree.root;
		while(treenode != null){
		    nextTreeNode = treenode.ns;
		    result.addChild(gNode.nodeID, treenode);
		    treenode = nextTreeNode;
		}
		// then add all the tree nodes in the right subtree
		treenode = rightResultTree.root;
		while(treenode != null){
		    nextTreeNode = treenode.ns;
		    result.addChild(gNode.nodeID, treenode);
		    treenode = nextTreeNode;
		}
		dtrList.put(gNode.nodeID);
		dtrList.concatenate(leftDtrList);
		dtrList.concatenate(rightDtrList);
		result.root.compressedNodes = dtrList;
		// make the list empty since all the dtrs are accounted for in this current node
		dtrList = new List();
	    }
	    // otherwise if this node is not a root node, 
	    else{
		// add its id to te dtrlist
		dtrList.put(gNode.nodeID);
		// point to the root of the first tree in the link
		treenode = leftResultTree.root;
		// go the last root node in the link
		while(treenode.ns != null){
		    treenode = treenode.ns;
		}
		// once you get to the end of the link of the left (linked) subtrees, link it to the right (linked) subtrees
		treenode.ns = rightResultTree.root;
		result = leftResultTree;
		dtrList.put(gNode.nodeID);
		dtrList.concatenate(leftDtrList);
		dtrList.concatenate(rightDtrList);
	    }
	}
	resultList.put(result);
	resultList.put(dtrList);
	return(resultList);
    }
    
    
    public int parseInputFile(String fileName, String sentence)
    {
	String token;
	int indexOfColon, numTokens, i=0;
	String line;
	String nodeID="";
	// stuff to be stored in the hash tables
	String nodeData,treeName, rootfoot, root, foot, nodeLabel, node_type, nodePos="";
	String dtrs="";
	String nodeInfo="";
	int nodeCount=0;
	// create a filestream
	FileInputStream f; 
	BufferedReader input;
	StringTokenizer tokenizer, nodeInfoTokens;
	int index=0;
	int count=0;
	int f_index=-1;
	String sent;
	String feat = null;
	List featList=null;
	boolean inserted;
	
	unification=false;
	
	try{
	    // open a stream to read the input file
	    f = new FileInputStream(fileName);
	    input = new BufferedReader( new InputStreamReader(f));
	    // find the part of the file that contains the graph for "sentence"
	    line = input.readLine();
	    while(line != null){
		if(line.startsWith("begin")){
		    sent = line.substring(line.indexOf("\"")+1, line.lastIndexOf("\""));
		    if(sent.compareTo(sentence) == 0){
			break;
		    }
		}
		line = input.readLine();
	    }
	    // once you have found the appropriate graph, read it in.
	    line = input.readLine();
	    
	    while(line.compareTo("end") != 0){
		//  get the features
		f_index=line.indexOf("<>[");
		if(f_index !=-1){
		    feat = line.substring(f_index,line.lastIndexOf("]")); 
		    feat = feat.substring(feat.indexOf("[")+1);
		    tokenizer = new StringTokenizer(feat, ";");
		    int featCount= tokenizer.countTokens();
		    int k =0;
		    feat="";
		    featList = new List();
		    for(k=0; k < featCount; k++){  featList.put(tokenizer.nextToken()); }
		    line=line.substring(0,f_index) + line.substring(line.lastIndexOf(">")+1);
		    // indicate that the features are unified
		    unification=true;
		}
		else if((f_index=line.indexOf("<#")) != -1){
		    feat = line.substring(f_index,line.lastIndexOf(">")+1); 
		    featList = new List();
		    featList.put(feat);
		    line=line.substring(0,f_index) + line.substring(line.lastIndexOf(">")+1);
		    // indicate that the features are NOT unified
		    unification=false;
		}
		else if((f_index=line.indexOf("[<")) !=-1){
		    //System.out.println("Warning: Graph does not seem to have features where expected.");
		    feat = line.substring(f_index,line.lastIndexOf(">")+1); 
		    featList = new List();
		    featList.put(feat);
		    line=line.substring(0,f_index) + line.substring(line.lastIndexOf(">")+1);
		    // indicate that the features are NOT unified
		    //unification=false;
		}
		tokenizer = new StringTokenizer(line, " ");
		count = tokenizer.countTokens();
		token = tokenizer.nextToken();
		// if this is a start node, there are only 2 tokens
		if(token.startsWith("start")){
		    nodeID = token.substring(0, token.indexOf(":"));
		    dtrs = tokenizer.nextToken();
		    nodeID=nodeID + "_" + startNumber; 
		    ++startNumber; 
		    graph.put(nodeID, dtrs);
		    graph.put(nodeID, new graphNode(nodeID, dtrs, "", "" , "", "", "", "START", null, false));
		}
		else{ // the string is of the form <nodeID:> <TreeInfo> [DaughterPair List]
		    if(count == 3){
			nodeID = token.substring(0,token.indexOf(":"));
			nodeInfo = tokenizer.nextToken();
			dtrs = tokenizer.nextToken(); 
		    }
		    if(count == 2){ // the node has no daughter list, i.e. it is a leaf
			nodeID = token.substring(0,token.indexOf(":"));
			nodeInfo = tokenizer.nextToken();
			dtrs = "[" + leaf;
		    }
		    // tokenize the node info into trees, node label and position
		    nodeInfoTokens = new StringTokenizer(nodeInfo, "<");
		    // get the treeName
		    treeName=nodeInfoTokens.nextToken();
		    // get the data containing the position
		    nodeData=nodeInfoTokens.nextToken();
		    nodeData=nodeData.substring(0, nodeData.indexOf(">"));
		    // get the node label and position from the node data
		    nodeLabel=nodeData.substring(0,nodeData.indexOf("/"));
		    nodePos= nodeData.substring(nodeData.indexOf("/")+1);
		    // get the type of demacartion being used
		    SUBSCRIPT_MARKER= nodeInfoTokens.nextToken();
		    SUBSCRIPT_MARKER = SUBSCRIPT_MARKER.substring(0, SUBSCRIPT_MARKER.lastIndexOf(">"));
		    // get node type
		    node_type = nodeInfoTokens.nextToken();
		    node_type = node_type.substring(0, node_type.lastIndexOf(">"));
		    // get the information about the root and foot
		    rootfoot=nodeInfoTokens.nextToken();
		    rootfoot= rootfoot.substring(0, rootfoot.indexOf(">"));
		    // get the root
		    root = rootfoot.substring(0, rootfoot.indexOf("/"));
		    index=rootfoot.lastIndexOf("/");
		    // if there is no foot, set the foot to ""
		    if(index+1 == rootfoot.length()){   foot=""; }
		    // otherwise get the foot
		    else{  foot = rootfoot.substring(rootfoot.indexOf("/")+1); }
		    inserted=isInserted(nodeLabel);
		    gNode = new graphNode(nodeID, dtrs, treeName, nodeLabel, nodePos, root, 
					  foot, node_type, featList, inserted);
		    graph.put(nodeID, gNode);
		    // if this graph contains inserted nodes, enable the menu item to display inserted nodes
		    if(inserted){  artificialNodesPresent=true;  }
		}
		line = input.readLine();
	    } // end inner while 
	} // end try
	catch(Exception e) {  e.printStackTrace(); return(0); }
	return 1;
    }
    
}
