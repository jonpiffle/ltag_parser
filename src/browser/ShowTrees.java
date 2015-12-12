import javax.swing.*;
import javax.swing.border.*;
import java.awt.event.*;
import javax.swing.event.*;
import java.io.*; 
import java.util.*;
import java.lang.*;
import java.awt.*;

public class ShowTrees extends JFrame
{
    int HORIZONTAL_SIZE,  VERTICAL_SIZE, depth,  MAXWIDTH, MAXHEIGHT;
    int sentenceCount = 0;
    //Tree tree; 
    ScrollPane derivedTreeScrollpane,derivationTreeScrollpane;
    Frame derivedTreeFrame, derivationTreeFrame ;
    static DrawTree drawDerivedTree = null;
    static DrawTree drawDerivationTree = null;
    static String inputFileName, outputFileName;
    static boolean showFeatures=false;
    ClientWindow  clientWindow=null;
    ClientWindowThread clientWindowThread;
    ParserConfigWindow  parserOptionsWindow;
    Graphics g;
    String filename;
    String currentSentence = null;
        
    boolean treesLoaded, graphLoaded;
    static boolean collapseNodes, expandNodes;
    static boolean parserLoaded, removeInserted;
    JList sentenceList=null;
    // menubar
    JMenuBar menubar;
    //menus
    JMenu file, options, show, forestFormat, formatFonts, fontTypes, fontSize, fontStyle, parser, view, featureOptions;
    // Checkbox items for parser options
    JCheckBox lowerCase, remPunct, useGram, unifyFeat, featBin, featLex, featGram, featPreProc;
    JTextField gramFile,featBinFile,featLexFile,featGramFile,featProcBin, maxFeatNum;
    // menuitems
    JMenuItem open, saveDerivation, saveGraphForSelectedSentence, saveGraphAs, close, quit, font, loadParser, closeParser, parserOptions, setFeatNum;
    JMenuItem omitFeat, showDerived, showDerivation, showAllFeatures, expandAllNodes,showArtificialNodes; 
    
    // checkbox menuitems
    JCheckBoxMenuItem xtagFormat, treeBankFormat;
    JScrollPane jListScrollpane;
    String longestSentence="";
    static DerivedTrees dtrees;
    String italicsText, plainText, boldText, boldAndItalicText;
    static Font currentFont;
    JTextArea textWindow;
    JScrollPane textWindowScroll;
    static int maxFeat = -1;
    static String serverCommand;
    int DERIVEDTREE, DERIVATIONTREE;
    JButton gramFileButton, featBinFileButton,featLexFileButton,featGramFileButton,featProcBinButton, parserOk, parserCancel;
        
    public ShowTrees()
    {
	super();
	setVisible(false);
	setTitle("Show Trees");
	MAXWIDTH = 1000;
	MAXHEIGHT = 1000;
	DERIVEDTREE=0;
	DERIVATIONTREE=1;
	// get the current font 
	currentFont = getContentPane().getFont();
	// font styles
	plainText = "Plain";
	boldText = "Bold";
	italicsText = "Italics";
	boldAndItalicText = "Bold & Italics";
	removeInserted=true;
	menubar     = new JMenuBar();
	file        = new JMenu("File");
	view        = new JMenu("View");
	options     = new JMenu("Options");
	parser      = new JMenu("Parser");
	formatFonts = new JMenu("Fonts");
	fontTypes   = new JMenu("Type");
	fontSize    = new JMenu("Size");
	fontStyle   = new JMenu("Style");
	forestFormat = new JMenu("Forest");
	
	featureOptions = new JMenu("Features");
	setFeatNum     = new JMenuItem("Set Number of Features Displayed");
	omitFeat       = new JMenuItem("Omit Features with Null Values");
	open           = new JMenuItem("Open Graph");
	saveDerivation           = new JMenuItem("Save Derivation");
	
	saveGraphAs= new JMenuItem("Save Complete Graph As");
	saveGraphForSelectedSentence = new JMenuItem("Save Graph for Selected Sentence");
	//saveDerivation.setEnabled(false);
	close          = new JMenuItem("Close");
	close.setEnabled(false);
	quit           = new JMenuItem("Quit");
	showDerived    = new JMenuItem("Show Derived Trees");
	showDerivation = new JMenuItem("Show Derivation Trees");
	showArtificialNodes = new JMenuItem("Show Artificial Nodes");
	showArtificialNodes.setEnabled(false);
	showAllFeatures= new JMenuItem("Show All Features");
	showAllFeatures.setEnabled(false);
	expandAllNodes    = new JMenuItem("Expand All Nodes");
	xtagFormat = new JCheckBoxMenuItem("Xtag Format");
	treeBankFormat = new JCheckBoxMenuItem("Tree Bank Format");
	xtagFormat.setState(true);
	treeBankFormat.setState(false);
	
	loadParser = new JMenuItem("Open Parser");
	closeParser = new JMenuItem("Close Parser");
	parserOptions= new JMenuItem("Parser");
	
	// get the graphics environment of current system
	GraphicsEnvironment gEnv = GraphicsEnvironment.getLocalGraphicsEnvironment();
	// get all the font names available on this system
	String envFonts[] = gEnv.getAvailableFontFamilyNames();
	int numFonts =envFonts.length;
	// create a menu item for each fontname
	FontTypeListener fontTypeListener = new FontTypeListener();
	for(int c=0; c < numFonts; c++){
	    font = new JMenuItem();
	    font.setText(envFonts[c]);
	    font.addActionListener(fontTypeListener);
	    fontTypes.add(font);
	}
	FontSizeListener fontSizeListener = new FontSizeListener();
	for(int c= 6 ; c < 60 ; c+=2){
	    font = new JMenuItem();
	    font.setText((new Integer(c)).toString());
	    font.addActionListener(fontSizeListener);
	    fontSize.add(font);
	}
	String styles[] = {plainText,boldText, italicsText, boldAndItalicText };
	FontStyleListener fontStyleListener = new FontStyleListener();
	for(int c=0; c <=3; c++ ){
	    font = new JMenuItem();
	    font.setText(styles[c]);
	    font.addActionListener(fontStyleListener);
	    fontStyle.add(font);
	}
	
	file.add(open);
	//file.add(saveGraphAs);
	//file.add(saveGraphForSelectedSentence);
	file.add(saveDerivation);
	file.add(close);
	file.add(quit);
	
	view.add(showDerived);
	view.add(showDerivation);
	view.add(showAllFeatures);
	view.add(showArtificialNodes);
	view.add(expandAllNodes);
	
	parser.add(loadParser);
	parser.add(closeParser);
	formatFonts.add(fontTypes);
	formatFonts.add(fontSize);
	formatFonts.add(fontStyle);
	
	forestFormat.add(treeBankFormat);
	forestFormat.add(xtagFormat);
	
	featureOptions.add(omitFeat);
	featureOptions.add(setFeatNum);
	
	options.add(parserOptions);
	options.add(formatFonts);
	options.add(forestFormat);
	options.add(featureOptions);
	
	menubar.add(file);
	menubar.add(view);
	menubar.add(parser);
	menubar.add(options);
	
	setJMenuBar(menubar);
	
	addWindowListener(new WindowActionListener());
	SymAction lSymAction = new SymAction();
	loadParser.addActionListener(lSymAction);
	closeParser.addActionListener(lSymAction);
	parserOptions.addActionListener(lSymAction);
	open.addActionListener(lSymAction);
	saveGraphAs.addActionListener(lSymAction);
	saveGraphForSelectedSentence.addActionListener(lSymAction);
	saveDerivation.addActionListener(lSymAction);
	close.addActionListener(lSymAction);
	quit.addActionListener(lSymAction);
	showDerived.addActionListener(lSymAction);
	showDerivation.addActionListener(lSymAction);
	showArtificialNodes.addActionListener(lSymAction);
	showAllFeatures.addActionListener(lSymAction);
	expandAllNodes.addActionListener(lSymAction);
	setFeatNum.addActionListener(lSymAction);
	omitFeat.addActionListener(lSymAction);
	treeBankFormat.addActionListener(lSymAction);
	xtagFormat.addActionListener(lSymAction);
    }
    
    public ShowTrees(String title)
    {
	this();
	setTitle(title);
	HORIZONTAL_SIZE = 400;
	VERTICAL_SIZE = 300;
	setSize(HORIZONTAL_SIZE, VERTICAL_SIZE);
	treesLoaded=graphLoaded=expandNodes=collapseNodes=false;
	jListScrollpane = new JScrollPane();
	getContentPane().add(jListScrollpane);
	pack();
    }
    
    public void setVisible(boolean b){
	if(b){ setLocation(50, 50); }
	super.setVisible(b);
    }
    
    static public void main(String args[])
    {
	int countTokens;
	ShowTrees mainFrame;
	String serverPath;
	String serverOption;
	String portNum;
	File checkFile;
	//String server=args[0];
	//StringTokenizer tokenizer = new StringTokenizer(server, " ");
	
	mainFrame = new ShowTrees("Show Trees");
	mainFrame.setVisible(true);
	
    }
    
   
    
    class WindowActionListener extends java.awt.event.WindowAdapter
    {
	public void windowClosing(java.awt.event.WindowEvent event){
	    String label;
	    Object object = event.getSource();
	    if(object == ShowTrees.this){ System.exit(0); }
	    else if(object == derivedTreeFrame){
		label = showDerived.getText();
		label = "Show" + label.substring(label.indexOf(" "));
		derivedTreeFrame.setVisible(false);
		showDerived.setText(label);
	    }
	    else if(object == derivationTreeFrame){
		label = showDerivation.getText();
		label = "Show" + label.substring(label.indexOf(" "));
		derivationTreeFrame.setVisible(false); 
		showDerivation.setText(label);
	    }
	}
    }
    
    class SymAction implements java.awt.event.ActionListener
    {
	public void actionPerformed(java.awt.event.ActionEvent event)
	{
	    Object object = event.getSource();
	    if (object == open){  openEventHandler(0); }
	    else if(object == loadParser){      loadParserActionHandler(event); }
	    else if(object == closeParser){     closeParserActionHandler(event); }
	    else if(object==parserOptions){     parserOptionsEventHandler(event); }
	    else if(object==saveGraphAs){       saveGraphAsActionHandler(); }
	    else if(object==saveGraphForSelectedSentence){  saveGraphForSentEventHandler();  }
	    else if(object == saveDerivation){  saveDerivationActionHandler();           }
	    else if(object == close){           close_ActionPerformed(event);       }
	    else if(object == quit){            quit_ActionPerformed(event);        }
	    else if(object == setFeatNum){      setFeatNumEventHandler(event); }
	    else if(object == omitFeat){        omitFeatActionHandler(event); }
	    else if(object == showDerived){     showDerivedTreeHandler(event);}
	    else if(object == showDerivation){  showDerivationTreeHandler(event);  }
	    else if(object == showArtificialNodes){  showArtificialEventHandler(); }
	    else if(object == showAllFeatures){     showAllFeaturesHandler(event);  }
	    else if(object == expandAllNodes){     expandAllNodesHandler(event); }
	    else if(object == treeBankFormat){  treeBankFormatEventHandler() ;}
	    else if(object == xtagFormat){      xtagFormatEventHandler();  }
	}
    }
    
    
    public void showDerivedTreeHandler(ActionEvent event){
	String label;
	setCursor(new Cursor(Cursor.WAIT_CURSOR));
	label = showDerived.getText();
	if(label.startsWith("Hide")){
	    label = "Show" + label.substring(label.indexOf(" "));
	    if(derivedTreeFrame != null){ derivedTreeFrame.setVisible(false); }
	}
	else{
	    label = "Hide" + label.substring(label.indexOf(" "));
	    if(dtrees != null){  displayDerivedTree(); }
	}
	showDerived.setText(label);
	setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
    }
    
    public void showDerivationTreeHandler(ActionEvent event){
	String label;
	setCursor(new Cursor(Cursor.WAIT_CURSOR));
	label = showDerivation.getText();
	if(label.startsWith("Hide")){
	    label = "Show" + label.substring(label.indexOf(" "));
	    if(derivationTreeFrame != null){ derivationTreeFrame.setVisible(false); }
	}
	else{
	    label = "Hide" + label.substring(label.indexOf(" "));
	    if(dtrees != null){  displayDerivationTree(); }
	}
	showDerivation.setText(label);
	setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
    }
    

    public void showAllFeaturesHandler(ActionEvent event){
	int height;
	String label;
	setCursor(new Cursor(Cursor.WAIT_CURSOR));
	label = showAllFeatures.getText();
	if(label.startsWith("Hide")){
	    label = "Show" + label.substring(label.indexOf(" "));
	    showFeatures = false;
	}
	else if(label.startsWith("Show")){
	    label = "Hide" + label.substring(label.indexOf(" "));
	    showFeatures = true;
	}
	showAllFeatures.setText(label);
	if(dtrees != null){ 
	    if(drawDerivedTree !=null){
		drawDerivedTree.XDIST=50;
		drawDerivedTree.removeAll();
		drawDerivedTree.ready = false;
		drawDerivedTree.drawTree( drawDerivedTree.currentStartNode);
		drawDerivedTree.ready=true;
		drawDerivedTree.repaint();
		height = drawDerivedTree.getHeight();
		if(height < derivedTreeFrame.getHeight()){
		    derivedTreeFrame.setSize(derivedTreeFrame.getWidth(),height);
		    derivedTreeFrame.repaint();
		}
	    }
	}
	setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
    }

    public void showArtificialEventHandler(){
	int height;
	String label;
	setCursor(new Cursor(Cursor.WAIT_CURSOR));
	label = showArtificialNodes.getText();
	if(label.startsWith("Show")){
	    label = "Hide" + label.substring(label.indexOf(" "));
	    showArtificialNodes.setText(label);
	    removeInserted=false;
	}
	else {
	    label = "Show" + label.substring(label.indexOf(" "));
	    showArtificialNodes.setText(label);
	    removeInserted=true;
	}
	if(dtrees != null){ 
	    if(drawDerivedTree !=null){
		drawDerivedTree.XDIST=50;
		drawDerivedTree.removeAll();
		drawDerivedTree.ready = false;
		drawDerivedTree.drawTree( drawDerivedTree.currentStartNode);
		drawDerivedTree.ready=true;
		drawDerivedTree.repaint();
		height = drawDerivedTree.getHeight();
		if(height < derivedTreeFrame.getHeight()){
		    derivedTreeFrame.setSize(derivedTreeFrame.getWidth(),height);
		    derivedTreeFrame.repaint();
		}
	    }
	}
	setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
    }
    
    public void expandAllNodesHandler(ActionEvent event){
	String label;
	int height;
	
	label = expandAllNodes.getText();
	if(label.startsWith("Expand")){
	    label = "Collapse" + label.substring(label.indexOf(" "));
	    expandNodes=true;
	    collapseNodes=false;
	}
	else{
	    label = "Expand" + label.substring(label.indexOf(" "));
	    collapseNodes=true;
	    expandNodes=false;
	}
	expandAllNodes.setText(label);
	if(dtrees != null){ 
	    if(drawDerivedTree !=null){
		setCursor(new Cursor(Cursor.WAIT_CURSOR));
		drawDerivedTree.XDIST=50;
		drawDerivedTree.removeAll();
		drawDerivedTree.ready = false;
		drawDerivedTree.drawTree( drawDerivedTree.currentStartNode);
		drawDerivedTree.ready=true;
		drawDerivedTree.repaint();
		height = drawDerivedTree.getHeight();
		if(height < derivedTreeFrame.getHeight()){
		    derivedTreeFrame.setSize(derivedTreeFrame.getWidth(),height);
		    derivedTreeFrame.repaint();
		}
		setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
	    }
	}
    }
    
    public void omitFeatActionHandler(ActionEvent event){
	String label;
	label = omitFeat.getText();
	if(label.startsWith("Omit")){
	    label = "Include" + label.substring(label.indexOf(" "));
	    omitFeat.setText(label);
	    DrawTree.omitNullFeatures=true;
	}
	else{
	    label = "Omit" + label.substring(label.indexOf(" "));
	    omitFeat.setText(label);
	    DrawTree.omitNullFeatures=false;
	}
	if(dtrees != null){ 
	    if(drawDerivedTree !=null && showFeatures){
		setCursor(new Cursor(Cursor.WAIT_CURSOR));
		drawDerivedTree.removeAll();
		drawDerivedTree.ready = false;
		drawDerivedTree.drawTree( drawDerivedTree.currentStartNode);
		drawDerivedTree.ready=true;
		drawDerivedTree.repaint();
		setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
	    }
	}
    }
    
    public void treeBankFormatEventHandler(){
	if(treeBankFormat.getState()){
	    xtagFormat.setState(false);
	}
	else{
	      xtagFormat.setState(true);
	}
	if(dtrees != null){ 
	    if((new Error(this, 0, "Would you like to reload graph ?" )).showErrMsg() == 0 && currentSentence != null){
		sentenceSelected();
	    }
	}
    }
    public void xtagFormatEventHandler(){
	if(xtagFormat.getState()){
	    treeBankFormat.setState(false);
	}
	else{
	    treeBankFormat.setState(true);
	}
	if(dtrees != null){ 
	    if((new Error(this, 0, "Would you like to reload graph ?" )).showErrMsg() ==1 && currentSentence != null){
		sentenceSelected();
	    }
	}
    }
    
    public void setFeatNumEventHandler(ActionEvent event){
	// create a new dialog 
	FeatDialog featDialog = new FeatDialog(this, "Set Number of features to be Diplayed", currentFont);
	// open the dialog
	featDialog.showWindow();
	// once the dialog closes, get the value entered by the user
	if(featDialog.value == -1){}
	else{
	    maxFeat=featDialog.value;
	    // redraw the tree with the new maxFeat
	    if(dtrees != null){ 
		if(drawDerivedTree !=null){
		    setCursor(new Cursor(Cursor.WAIT_CURSOR));
		    drawDerivedTree.removeAll();
		    drawDerivedTree.ready = false;
		    drawDerivedTree.drawTree( drawDerivedTree.currentStartNode);
		    drawDerivedTree.ready=true;
		    drawDerivedTree.repaint();
		    setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
		}
		
	    }
	}
    }
    
    public void parserOptionsEventHandler(ActionEvent event){
	parserOptionsWindow = new ParserConfigWindow(this, "ParserOptions");
	// set the defaults
	parserOptionsWindow.setVisible(true);
	parserOptionsWindow.dispose();
	parserOptionsWindow=null;
    } 

    
    // open the file and read all the information in it an put it in a text box in a panel and add the panel to the frame
    public void openEventHandler(int caller)
    {
	JFileChooser fileChooser;
	FileInputStream f;
	File file;
	BufferedReader input;
	String line, sentence;
	Vector vector=null;
	int longestString=0;
	int width, height;
	FontMetrics fm;
	int lineNumber=1;
	int userChoice;
	sentenceCount=0;
	if(caller ==0){
	    fileChooser= new JFileChooser();
	    fileChooser.setDialogTitle("Open Graph");
	    userChoice = fileChooser.showDialog(this, "open");
	    if(userChoice == JFileChooser.APPROVE_OPTION){     filename = (fileChooser.getSelectedFile()).toString(); }
	    else{  return; } // user pressed cancel
	}
	else{}// filename has been set by parseInputActionHandler in ClientWindow.java
	// check that the file is a valid file
	file = new File(filename);
	if(file.isFile() && filename !=null){
	    sentenceList = new JList();
	    sentenceList.setAutoscrolls(true);
	    sentenceList.setVisibleRowCount(5);
	    sentenceList.addListSelectionListener(new ListAction()); 
	    sentenceList.setSelectionMode((new DefaultListSelectionModel()).SINGLE_SELECTION);
	    try{
		f = new FileInputStream(filename);
		input = new BufferedReader(new InputStreamReader(f));
		try{
		    line = input.readLine();
		    lineNumber++;
		    // create a new list that will display all the sentences in the graph
		    vector = new Vector(4,4);
		    while(line != null){
			if(line.startsWith("begin")){
			    // keep a count of the # of sentence (used to determine the height of the List that will be displayed
			    sentenceCount++;
			    sentence = line.substring(line.indexOf("\"") + 1, line.lastIndexOf("\""));
			    // add the sentence to the list
			    vector.add(sentenceCount + ". " + sentence);
			    // keep track of the longest sentence in terms of character (used to set the width of the list
			    if(sentence.length() > longestString){
				longestString = sentence.length();
				longestSentence = sentence;
			    }
			}
			lineNumber++;
			line = input.readLine();
		    }
		}
		catch(Exception e){
		    (new Error(this, 1, "Unable to read line " + lineNumber + " in file " + filename)).showErrMsg();
		    return; // return immediately without doing anything
		}
	    }
	    catch(Exception e){
		(new Error(this, 1, "Unable to create File Input Stream for file " + filename)).showErrMsg();
		return; // return immediately without doing anything
	    }
	    if(sentenceCount > 0){
		close.setEnabled(true);
		sentenceList.setListData(vector);
		fm = getFontMetrics(getContentPane().getFont());
		height = fm.getHeight() * (sentenceCount+1);
		width = fm.stringWidth(longestSentence);
		if(height > MAXHEIGHT) { height = MAXHEIGHT; }
		jListScrollpane.setViewportView(sentenceList);
		// if there is only one sentence, just display it immediately
		if(sentenceCount==1){ sentenceList.setSelectedIndex(0);  sentenceSelected(); }
	    }
	}
	else{ 
	    (new Error(this, 1, "File: \"" + filename +"\" could not be found")).showErrMsg();
	}
    }
    
    public void loadParserActionHandler(ActionEvent event){
	//try{
	if(!parserLoaded){
	    /*
	      clientWindowThread = new ClientWindowThread(this);
	      if(clientWindowThread.initClientWindow() ==1){  
	      parserLoaded=true;
	      clientWindowThread.start(); }
	      else{  (new Error(this, 1, "Client could not be initialized. Make sure Parser Server is running")).showErrMsg(); }
	    */
	    
	    // create new client
	    clientWindow = new ClientWindow(this);
	    // initialize the client
	    if(clientWindow.initClientWindow() !=1){
		System.out.println("Error at openning client");
		// create error message indicating that client could not connect to server
		(new Error(this, 1, "Client could not be initialized. Make sure Parser Server is running")).showErrMsg();
	    }
	    else{  
		clientWindow.pack();
		clientWindow.setVisible(true);
		parserLoaded=true; 
	    }
	}
	else{  (new Error(this, 1, "Parser Already loaded")).showErrMsg(); }
	//}
	//catch(Exception e){
	//(new Error()).showErrMsg();
	//}
    }
    
    public void closeParserActionHandler(ActionEvent e){
	if(clientWindow != null && parserLoaded){
	    clientWindow.closeParserActionHandler();
	}
	clientWindow = null;
	/*if(clientWindowThread != null && parserLoaded){
	  clientWindowThread.stopClient();
	  }
	*/
    }
    

    /*
      Name:
      Args:
      Returns:
      Description:
           Opens a file save dialog with a given title (Save Graph, Save Derivation, etc)
	   Gets the the filename from the file dialog and returns it
     */
    

    public String getFileNameFromSaveDialog(String title){
	File checkFile;
	JFileChooser fileSaver;
	int errorReturnStatus;
	int userChoice;
	String filename;
	// first check to see that there is indeed a graph loaded, and that there is a tree (parse/derivation) showing
	fileSaver = new JFileChooser();
	fileSaver.setDialogTitle(title);
	userChoice = fileSaver.showSaveDialog(this);
	if(userChoice == JFileChooser.APPROVE_OPTION){
	    // check if the file already exists
	    filename=(fileSaver.getSelectedFile()).toString();
	    checkFile = new File(filename);
	    // prompt to overwrite
	    if(checkFile.isFile()){ 
		errorReturnStatus= (new Error(this, 0, "The file \""+filename+ "\" already exists. Overwrite it ?")).showErrMsg();
		if(errorReturnStatus==1){
		    return filename;
		}
		else if(errorReturnStatus==0){ filename = getFileNameFromSaveDialog(title); }
	    }
	}
	else{ filename = null; }  // user clicked cancel button
	return filename;  
    }
    
    
    /*
      Name:
      Args:
      Returns:
      Description:
          opens a file given by `fileName' and write `buffer' to the file
     */
    
    public void saveToFile(String buffer, String fileName){
	try{
	    DataOutputStream outputStream = new DataOutputStream(new FileOutputStream(fileName));
	    outputStream.writeBytes(buffer);
	    outputStream.close();
	}
	catch(Exception e){  
	    (new Error(this, 1, "Error: File could not be written")).showErrMsg();
	}
    }
    
    /*
      Name:
      Args:
      Returns:
      Description:
            writes the current graph to a buffer
	    and then saves it to a filename selected by the user
     */

    public void saveGraphAsActionHandler(){
	String filename, currentGraph;
	filename = getFileNameFromSaveDialog("Save Graph As...");
	if(filename !=null){
	    currentGraph = getCurrentGraph();
	    saveToFile(currentGraph, filename);
	}
	
    }
    
    public void saveDerivationActionHandler(){
	String derivation;
	// first check to see that there is indeed a graph loaded, and that there is a tree (parse/derivation) showing
	if(dtrees !=null && (drawDerivedTree !=null || drawDerivationTree !=null)){
	    inputFileName = getFileNameFromSaveDialog("Save Graph");
	    if(inputFileName !=null){
		derivation = getCurrentDerivation();
		saveToFile(derivation, inputFileName);
	    }
	}
	else{    (new Error(this, 1, "No Graph Loaded")).showErrMsg(); }
    }
    
    public void saveGraphForSentEventHandler(){
	
    }

    /*
      Name:
           getCurrentDerivation
      Args:
           void
      Returns:
             string representing a single string containing both the derived and derivation trees
      Description:
             reads the current tree and extracts all the necessary information from the
	     `graph' hashtable that is necessary to reconstruct the tree. This information 
	     is then written out as a string which is returned
     */
    
    public String  getCurrentDerivation(){
	String buffer="";
	buffer = "begin sent=\""+currentSentence + "\"\n";
	buffer = buffer + createBuffer("start_" + drawDerivedTree.currentStartNode);
	buffer = buffer + "end";
    	return buffer;
    }
    
    
    public String getCurrentGraph(){
	String currentGraph = "";
	return currentGraph;

    }

    public void close_ActionPerformed(ActionEvent event){
	if(dtrees != null){
	    // remove the scrollpane on which we draw object from the frame
	    if(derivedTreeFrame != null){    derivedTreeFrame.dispose();  }
	    if(derivationTreeFrame != null){  derivationTreeFrame.dispose(); }
	    graphLoaded = false;
            treesLoaded = false;
            filename = null;
	    // if there was a derived trees object created, set this object
            dtrees = null;
	}
	if(sentenceList !=null){
	    jListScrollpane.remove(sentenceList);
	    sentenceList.setVisible(false);
	    sentenceList = null;
	    repaint();
	}
	sentenceCount=0;
	close.setEnabled(false);
    }
    
    public void quit_ActionPerformed(ActionEvent event){
        System.exit(0);    
    }
    
    // this class is used to listen to events that are generated
    // by the menu items in Fonts->Types ONLY.
    
    class FontTypeListener implements java.awt.event.ActionListener
    {
	public void actionPerformed(ActionEvent e){
	    String fontName;
	    // get the menuitem responsible for the event
	    JMenuItem menuItem = (JMenuItem)e.getSource();
	    // get the text from the menu
	    fontName = menuItem.getText();
	    // create a new font with the size and style of the new font
	    currentFont = new Font(fontName, currentFont.getStyle(), currentFont.getSize());
	    // set the currentFont to be the new font
	    getContentPane().setFont(currentFont);
	    updateFonts(currentFont);
	}
    }
    // this class is used to listen to events that are generated
    // by the menu items in Fonts->Size ONLY.
    
    class FontSizeListener implements java.awt.event.ActionListener
    {
	public void actionPerformed(ActionEvent e){
	    String fontSize;
	    // get the menuitem responsible for the event
	    JMenuItem menuItem = (JMenuItem)e.getSource();
	    // get the text from the menu
	    fontSize = menuItem.getText();
	    Integer integer = new Integer(fontSize);
	    int size = integer.intValue();
	    // create a new font with the size and style of the new font
	    currentFont = new Font(currentFont.getFontName(), currentFont.getStyle(), size);
	    // set the currentFont to be the new font
	    getContentPane().setFont(currentFont);
	    updateFonts(currentFont);
	    
	}
	
    }

    class FontStyleListener implements java.awt.event.ActionListener
    {
	public void actionPerformed(ActionEvent e){
	    String fontStyle;
	    // get the menuitem responsible for the event
	    JMenuItem menuItem = (JMenuItem)e.getSource();
	    // get the text from the menu
	    fontStyle = menuItem.getText();
	    if(fontStyle.compareTo(boldAndItalicText)==0){  currentFont = new Font(currentFont.getFontName(), Font.BOLD, currentFont.getSize()); }
	    else if(fontStyle.compareTo(plainText)==0){  currentFont = currentFont.deriveFont(Font.PLAIN); }
	    else if(fontStyle.compareTo(boldText)==0){   currentFont = currentFont.deriveFont(Font.BOLD); }
	    else if(fontStyle.compareTo(italicsText)==0){ currentFont = currentFont.deriveFont(Font.ITALIC); }
	    // set the currentFont to be the new font
	    getContentPane().setFont(currentFont);
	    updateFonts(currentFont);
	}
    }
    
    public void updateFonts(Font currentFont)
    {
	String label1, label2;
	
	// Update the fonts of the sentenceList component
	FontMetrics f = getContentPane().getFontMetrics(currentFont);
	if(sentenceList != null){
	    sentenceList.setFont(currentFont);
	    int height = f.getHeight() * sentenceCount;
	    int width = f.stringWidth(longestSentence);
	    if(height > sentenceList.getHeight() && width > sentenceList.getWidth()){
		jListScrollpane.setSize(width + 30, height+30);
		setSize(width+80, height + 100);
	    }
	    else if(height > sentenceList.getHeight()){
		jListScrollpane.setSize(jListScrollpane.getWidth(), height+30);
		setSize(getWidth(), height + 100);
	    }
	    else if(width > sentenceList.getWidth()){
		jListScrollpane.setSize(width + 30,jListScrollpane.getHeight());
		setSize(width+80, getHeight());
	    }
	    repaint();
	}
	label1 = showDerived.getText();
	label2 = showDerivation.getText();
	if(dtrees !=null){
	    // update the fonts of "Show Derivated Trees" if any 
	    if(label1.startsWith("Hide")){
		// if there is a tree showing, remove all the components and redraw them with the new fonts
		if(drawDerivedTree !=null){
		    drawDerivedTree.removeAll();
		    drawDerivedTree.setFont(currentFont);
		    drawDerivedTree.fontmetrics= f;
		    drawDerivedTree.ready = false;
		    drawDerivedTree.drawTree( drawDerivedTree.currentStartNode);
		    drawDerivedTree.ready=true;
		    drawDerivedTree.repaint();
		}
	    }
	    // update the fonts of "Show Derivation Trees if any
	    if(label2.startsWith("Hide")){
		if(drawDerivationTree !=null){
		    drawDerivationTree.removeAll();
		    drawDerivationTree.setFont(currentFont);
		    drawDerivationTree.fontmetrics= f;
		    drawDerivationTree.ready = false;
		    drawDerivationTree.drawTree( drawDerivedTree.currentStartNode);
		    drawDerivationTree.ready=true;
		    drawDerivationTree.repaint();
		}
	    }
	}
    }
    
   
    
    class ListAction implements ListSelectionListener{
	public void valueChanged(ListSelectionEvent e){
	    sentenceSelected();
	}
    }
   
    public void sentenceSelected(){
	String showDerivedLabel, showDerivationLabel;
	// get the selected sentence
	try{
	    // set busy cursor
	    setCursor(new Cursor(Cursor.WAIT_CURSOR));
	    currentSentence = (String)sentenceList.getSelectedValue();
	    currentSentence = currentSentence.substring(currentSentence.indexOf(".") + 2);
	    dtrees = new DerivedTrees(this);
	    dtrees.XTAG_TREES = xtagFormat.getState();
	    dtrees.TREEBANK_TREES = treeBankFormat.getState();
	    // call parseInputFile with the filename and the sentence selected by the user
	    if(dtrees.parseInputFile(filename, currentSentence) != 0){
		// if features are unified, enable the display features menuitem
		if(dtrees.unification){ 
		    showAllFeatures.setEnabled(true); 
		    featureOptions.setEnabled(true);
		}
		else{  
		    showAllFeatures.setEnabled(false);   
		    featureOptions.setEnabled(false);
		}
		// enable menu items for displaying artificial nodes if they are present
		if(dtrees.artificialNodesPresent){  showArtificialNodes.setEnabled(true);  }
		else{   showArtificialNodes.setEnabled(false); }
		
		showDerivedLabel = showDerived.getText();
		showDerivationLabel = showDerivation.getText();
		if(showDerivedLabel.startsWith("Show") && showDerivationLabel.startsWith("Show")){
		    showDerived.setText("Hide" + showDerivedLabel.substring(showDerivedLabel.indexOf(" ")));
		}
		if(showDerivedLabel.startsWith("Hide")){  displayDerivedTree();  }
		if(showDerivationLabel.startsWith("Hide")){ displayDerivationTree();  }
	    }
	    else{ 
		(new Error(this, 1, "Error: Cannot parse input file: " + filename)).showErrMsg();
	    }
	}
	catch(Exception exception){ exception.printStackTrace(); }
	setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
    }

    /*
      Name:
           displayDerivedTree
      Args:
          void
      Returns:
          void
      Description:
         
     */
    
    
    public void displayDerivedTree(){
	List derivedDataList;
	int X = 50;
	int Y=50;
	int height, width;
	
	// compute the derived trees
	derivedDataList= dtrees.computeDerived();
	if(derivedDataList == null){
	    return;
	}
	// create a drawTree object to draw the derived trees;    
	drawDerivedTree = new DrawTree(DERIVEDTREE);
	drawDerivedTree.data = derivedDataList;
	//drawDerivedTree.initTrees();
	drawDerivedTree.drawTree(0);
	drawDerivedTree.ready = true;
	width = drawDerivedTree.currentX + 50;
	height = drawDerivedTree.treeDepth + 50;
	if(width > MAXWIDTH){     width = MAXWIDTH; }
	if(height > MAXHEIGHT){   height = MAXHEIGHT; }
	// if there is no existing frame, create a new one, otherwise recycle the existing one
	if(derivedTreeFrame == null){
	    // create a new scrollpane to hold the drawing and to enable scrolling
	    derivedTreeScrollpane =  new ScrollPane(ScrollPane.SCROLLBARS_AS_NEEDED);
	    derivedTreeScrollpane.add(drawDerivedTree);
	    derivedTreeScrollpane.setBounds(0,50,width, height);
	    // create the frame
	    derivedTreeFrame = new Frame();
	    derivedTreeFrame.addWindowListener(new WindowActionListener());
	    derivedTreeFrame.setTitle("Derived Trees");
	    derivedTreeFrame.setBounds(X, Y, width, height);
	    // add the scroll pane to the frame
	    derivedTreeFrame.add(derivedTreeScrollpane);
	}
	else{
	    derivedTreeScrollpane.removeAll();
	    if(width >  derivedTreeFrame.getWidth() || height > derivedTreeFrame.getHeight()){
		derivedTreeFrame.setSize(width, height);
	    }
	    derivedTreeScrollpane.add(drawDerivedTree);
	}
	derivedTreeFrame.pack();
	derivedTreeFrame.setVisible(true); 
    }

    public void displayDerivationTree(){
	int X = 50;
	int Y=50;
	int height, width;
	Rectangle rect;
	List derivationDataList;
	derivationDataList = dtrees.computeDerivation();
	if(derivationDataList == null){
	    return;
	}
	drawDerivationTree = new DrawTree(DERIVATIONTREE);
	drawDerivationTree.data = derivationDataList;
	//drawDerivationTree.initTrees();
	drawDerivationTree.drawTree(0);
	drawDerivationTree.ready = true;
	width = drawDerivationTree.currentX + 50;
	height = drawDerivationTree.treeDepth + 50;
	if(width > MAXWIDTH){    width = MAXWIDTH; }
	if(height > MAXHEIGHT){  height = MAXHEIGHT; }
	if(derivationTreeFrame == null){
	    derivationTreeScrollpane = new ScrollPane(ScrollPane.SCROLLBARS_AS_NEEDED);
	    derivationTreeScrollpane.setBounds(0,50,width, height);
	    derivationTreeScrollpane.add(drawDerivationTree);
	    derivationTreeFrame = new Frame();
	    derivationTreeFrame.addWindowListener(new WindowActionListener());
	    derivationTreeFrame.setTitle("Derivation Trees");
	    derivationTreeFrame.setBounds(X,Y,width, height);
	    derivationTreeFrame.add(derivationTreeScrollpane);
	}
	else{
	    derivationTreeScrollpane.removeAll();
	    if(width >  derivationTreeFrame.getWidth() || height > derivationTreeFrame.getHeight()){
		derivationTreeFrame.setSize(width, height);
	    }
	    derivationTreeScrollpane.add(drawDerivationTree);
	}
	derivationTreeFrame.pack();
	derivationTreeFrame.setVisible(true);
    }
    
    public String createBuffer(String nodeID){
	boolean leaf=false;
	String  left, right, dtrPairList, leftBuffer, rightBuffer, dtrPair;
	String feat;
	String buffer="";
	graphNode gNode = (graphNode)dtrees.graph.get(nodeID);
	if(nodeID.compareTo("(nil)")==0){
	    return buffer;
	}
	if(nodeID.startsWith("start")){
	    buffer = "start: " + gNode.daughters + "\n";
	    buffer = buffer + createBuffer(gNode.daughters);
	    return buffer;
	}
	buffer = nodeID + ": " + gNode.treeName + "<" + gNode.nodeLabel + "/" + gNode.position + "><-><" 
	    + gNode.nodeType + "><" + gNode.rootLabel + "/" + gNode.footLabel + ">" ;
	// check to see if this node has altered the daughter pairs being displayed
	if(dtrees.altered.containsKey(nodeID)){
	    dtrPair=(String)dtrees.altered.get(nodeID);
	}
	else{
	    dtrPairList=gNode.daughters;
	    if(dtrPairList.compareTo("[" + dtrees.leaf)==0){
		dtrPair=dtrPairList.substring(dtrPairList.indexOf("[")+1);
		// check if features are unified
		if(dtrees.unification){
		    if(gNode.features !=null){
			ListNode l = gNode.features.head;
			feat="<>[";
			while(l.next !=null){
			    feat=feat + ";" + (String)l.next.data;
			    l=l.next;
			}
			feat=feat+"]";
			buffer=buffer + feat + "\n";
		    }
		}
		else{ buffer=buffer+"["+(String)gNode.features.head.next.data+"]" + "\n"; }
		leaf=true;
	    }
	    else{
		dtrPair=dtrPairList.substring(dtrPairList.indexOf("[")+1,dtrPairList.indexOf("]"));
	    }
	}
	if(!leaf){
	    left = dtrPair.substring(0,dtrPair.indexOf(","));
	    right = dtrPair.substring(dtrPair.indexOf(",")+1);
	    leftBuffer=createBuffer(left);
	    rightBuffer=createBuffer(right);
	    if(dtrees.unification){	    buffer = buffer + "<> [" + dtrPair + "]\n"; }
	    else{  buffer = buffer + "[] [" + dtrPair + "]\n"; }
	    buffer = buffer + leftBuffer + rightBuffer;
	}
	return buffer;
    }
}

