import javax.swing.*;
import javax.swing.border.*;
import java.awt.event.*;
import javax.swing.event.*;
import java.io.*; 
import java.util.*;
import java.lang.*;
import java.awt.*;

/*
  Name:
      ServerConfigWindow
  Description
      Creates a user interface to allow user to configure server
      
 */


public class ServerConfigWindow extends JDialog
{
    Container container;
    JPanel grammar, features, confirm, parser;
    JTabbedPane tabbedPane;
    // checkbox items for activating file setting textboxes
    JCheckBox selectGrammar,  featBin, featLex, featGram, featPreProc; 
    JCheckBox selectDeleteFilter, selectNbestParser, selectParser, selectSynGetBin;
    // textfields for entering filenames for different files used by parser
    JTextField gramFile,featBinFile,featLexFile,featGramFile,featProcBin;
    JTextField parserField, nbestParserField,synGetBinField, deleteFilterField ;
    // button for browsing files when setting paths for files used by parser
    JButton gramFileButton, featBinFileButton,featLexFileButton,featGramFileButton,featProcBinButton;
    JButton parserBrowseButton, nbestParserBrowseButton,deleteFilterBrowseButton ,synGetBinBrowseButton;
    // buttons for dismissing configuration window
    JButton parserOk, parserCancel;
    // default path to preferences
    //static String pathNamesFile="/mnt/linc/xtag/work/moses/ParserServer/server.paths";
    static String pathNamesFile="/home/kimanzi/ParserServer/server.paths";
    // file names
    String grammarFileName, parserBinFileName, nbestParserBinFileName, featBinFileName;
    String featLexFileName, featGramFileName, featProcBinFileName, synGetBinFileName, deleteFilterBinFileName;
    // Parameters for Configurable pathnames
    String grammar_file="grammar_file";
    String parser_bin="parser_bin";
    String nbestparser_bin="nbestparser_bin";
    String synget_bin="synget_bin";
    String deletefilter_bin="deletefilter_bin";
    String checker="checker";
    String lexfeat_file="lexfeat_file";
    String gramfeat_file="gramfeat_file";
    String feat_postprocessor="feat_postprocessor";
    // grammar labels
    String selectGrammarLab;
    // features labels
    String featBinLab, featLexLab,featGramLab, featPreProcLab;
    // parser binaries
    String parserLab, nbestParserLab;
    // others
    String synGetBinLab, deleteFilterLab;
    // tab descriptions
    String gramDesc,featDesc, parserDesc, browse;
    
    public ServerConfigWindow(JFrame frame, String title){
	super(frame, title, true);
	FontMetrics fm ;
	int height;
	int cols=30;
	// grammar labels
	selectGrammarLab = " use grammar file";
	// features labels
	featBinLab = " feature unifier binary";
        featLexLab = " lexical features filename";
	featGramLab = " grammar features filename";
	featPreProcLab = " feature postprocessor binary";
	// parser binary labels
	parserLab = " parser binary";
	nbestParserLab = " nbest parser binary";
	// other
	synGetBinLab = " syn_get_bin";
	deleteFilterLab = " delete filter";
	
	// tab descriptions
	gramDesc = "Specify specific grammar file";
	featDesc = "Configure Feature handling";
	parserDesc = "Set Paths to Parser Binaries";
	// create a listener for the bottons etc
	ParserActionListener listener = new ParserActionListener();
	browse = "Browse";
	container = getContentPane();
	container.setLayout(new BorderLayout());
	// create panels for each of the preference groups
	grammar = new JPanel(null);
	features  = new JPanel(null);
	parser = new JPanel(null);
	confirm = new JPanel();
		
	// set the default values
	setPaths();
	fm= getFontMetrics(container.getFont());
	height  = fm.getHeight()+5;
	//             **  Grammar Panel  **
        selectGrammar= new JCheckBox(selectGrammarLab, false);
	gramFile    = new JTextField(grammarFileName,cols);
	gramFile.setEnabled(false); 
	gramFileButton = new JButton(browse);
	gramFileButton.setEnabled(false); 
	// add actionlisteners
	selectGrammar.addActionListener(listener);
	gramFileButton.addActionListener(listener);
	// set the bounds 	
	selectGrammar.setBounds(20,20,fm.stringWidth(selectGrammarLab)+50, height);
	gramFile.setBounds(50, height+30, 400, height);
	gramFileButton.setBounds(470, height+30,fm.stringWidth(browse)+50 , height);
	// add components to the panel
	grammar.add(selectGrammar);
	grammar.add(gramFile);
	grammar.add(gramFileButton);
	
	//                 ** Feature Panel  **
	
	featBin = new JCheckBox(featBinLab,false);
	featLex = new JCheckBox(featLexLab,false);
	featGram    = new JCheckBox(featGramLab,false);
	featPreProc = new JCheckBox(featPreProcLab,false);
	// create text fields
	featBinFile = new JTextField(featBinFileName,cols);
	featLexFile = new JTextField(featLexFileName,cols);
	featGramFile= new JTextField(featGramFileName, cols);
	featProcBin = new JTextField(featProcBinFileName,cols);
	// initially disable all the textfields so that user is required to activate textfields before changing defaults
	featBinFile.setEnabled(false);
	featLexFile.setEnabled(false);
	featGramFile.setEnabled(false);
	featProcBin.setEnabled(false);
	// create browse buttons
	featBinFileButton = new JButton(browse);
	featLexFileButton = new JButton(browse);
	featGramFileButton = new JButton(browse);
	featProcBinButton = new JButton(browse);
	// disable the browse buttons until user activates specific option
	featBinFileButton.setEnabled(false);
	featLexFileButton.setEnabled(false);
	featGramFileButton.setEnabled(false);
	featProcBinButton.setEnabled(false);
	// register event handlers
	featBin.addActionListener(listener);
	featBinFileButton.addActionListener(listener);
	featGram.addActionListener(listener);
	featGramFileButton.addActionListener(listener);
	featPreProc.addActionListener(listener);
	featLex.addActionListener(listener);
	featLexFileButton.addActionListener(listener);
	featProcBinButton.addActionListener(listener);
	// set the bounds
	featBin.setBounds(20, 60, 400, 20);
	featBinFile.setBounds(40, 90, 400, 20);
	featBinFileButton.setBounds(460, 90, fm.stringWidth(browse)+50 , height);
	featLex.setBounds(20, 130, 400, 20);
	featLexFile.setBounds(40, 160, 400, 20);
	featLexFileButton.setBounds(460, 160, fm.stringWidth(browse)+50 , height);
	featGram.setBounds(20, 200, 400, 20);
	featGramFile.setBounds(40, 230, 400, 20);
	featGramFileButton.setBounds(460, 230, fm.stringWidth(browse)+50 , height);
	featPreProc.setBounds(20, 270, 400, 20);
	featProcBin.setBounds(40, 300, 400, 20);
	featProcBinButton.setBounds(460, 300, fm.stringWidth(browse)+50 , height);
	// add the components to the feature panel
	features.add(featBin);
	features.add(featBinFile);
	features.add(featLex);
	features.add(featLexFile);
	features.add(featGram);
	features.add(featGramFile);
	features.add(featPreProc);
	features.add(featProcBin);
	features.add(featBinFileButton);
	features.add(featLexFileButton);
	features.add(featGramFileButton);
	features.add(featProcBinButton);
	//  parser
	// checkboxes
	selectParser = new JCheckBox(parserLab, false);
	selectNbestParser = new JCheckBox(nbestParserLab, false);
	selectSynGetBin = new JCheckBox(synGetBinLab, false);
	selectDeleteFilter = new JCheckBox(deleteFilterLab, false);
	// text fields
	parserField= new JTextField(parserBinFileName,cols);
	nbestParserField = new JTextField(nbestParserBinFileName, cols);
	synGetBinField = new JTextField(synGetBinFileName, cols);
	deleteFilterField = new JTextField(deleteFilterBinFileName, cols);
       
	parserField.setEnabled(false); 
	nbestParserField.setEnabled(false);
	synGetBinField.setEnabled(false);
	deleteFilterField.setEnabled(false);
	
	parserBrowseButton = new JButton(browse);
	nbestParserBrowseButton = new JButton(browse);
	synGetBinBrowseButton = new JButton(browse);
	deleteFilterBrowseButton = new JButton(browse);
	
	parserBrowseButton.setEnabled(false); 
	nbestParserBrowseButton.setEnabled(false);
	synGetBinBrowseButton.setEnabled(false);
	deleteFilterBrowseButton.setEnabled(false);
	// add actionlisteners
	selectParser.addActionListener(listener);
	selectNbestParser.addActionListener(listener);
	selectSynGetBin.addActionListener(listener);
	selectDeleteFilter.addActionListener(listener);
	
	parserBrowseButton.addActionListener(listener);
	nbestParserBrowseButton.addActionListener(listener);
	synGetBinBrowseButton.addActionListener(listener);
	deleteFilterBrowseButton.addActionListener(listener);
	
	// set the bounds 	
	selectParser.setBounds(20,20,fm.stringWidth(selectGrammarLab)+50, height);
	selectNbestParser.setBounds(20, 130, 400, 20);
	selectSynGetBin.setBounds(20, 200, 400, 20);
	selectDeleteFilter.setBounds(20, 270, 400, 20);
				  
	parserField.setBounds(50, height+30, 400, height);
	nbestParserField.setBounds(40, 160, 400, 20);
	synGetBinField.setBounds(40, 230, 400, 20);
	deleteFilterField.setBounds(40, 300, 400, 20);

	parserBrowseButton.setBounds(470, height+30,fm.stringWidth(browse)+50 , height);
	nbestParserBrowseButton.setBounds(460, 160, fm.stringWidth(browse)+50 , height);
	synGetBinBrowseButton.setBounds(460, 230, fm.stringWidth(browse)+50 , height);
	deleteFilterBrowseButton.setBounds(460, 300, fm.stringWidth(browse)+50 , height);
	

	// add components to the panel
	parser.add(selectParser);
	parser.add(selectNbestParser);
	parser.add(selectSynGetBin);
	parser.add(selectDeleteFilter);
	parser.add(parserField);
	parser.add(nbestParserField);
	parser.add(synGetBinField);
	parser.add(deleteFilterField);
	
	parser.add(parserBrowseButton);
	parser.add(nbestParserBrowseButton);
	parser.add(synGetBinBrowseButton);
	parser.add(deleteFilterBrowseButton);
	// create a tabbed pane to contain punctuation, grammar, features and confirm panels
	tabbedPane = new JTabbedPane();
	tabbedPane.setSize(600,450);
	tabbedPane.addTab("Parser", null, parser, parserDesc);
	tabbedPane.addTab("Grammar", null, grammar,gramDesc); 
	tabbedPane.addTab("Features", null, features,featDesc); 
	// add the tabbed pane 
	container.add(tabbedPane, BorderLayout.CENTER);
	
	// confirm panel
	parserOk = new JButton("OK");
	parserCancel = new JButton("Cancel");
	
	parserOk.addActionListener(listener);
	parserCancel.addActionListener(listener);
	
	confirm.add(parserOk);
	confirm.add(parserCancel);
	
	container.add(confirm, BorderLayout.SOUTH);
	setSize(tabbedPane.getWidth()+30, tabbedPane.getHeight()+confirm.getHeight()+20);
	//setSize(700,500);
	//setResizable(false);
	
    } 

    class ParserActionListener implements ActionListener
    {
	public void actionPerformed(ActionEvent event){
	    Object object = event.getSource();
	    // check boxes
	    if(object == selectGrammar){       
		gramFile.setEnabled(selectGrammar.isSelected()); 
		gramFileButton.setEnabled(selectGrammar.isSelected()); 
	    }
	    else if(object == selectParser){
		parserField.setEnabled(selectParser.isSelected());
		parserBrowseButton.setEnabled(selectParser.isSelected());
	    }
	    else if(object == selectNbestParser){
		nbestParserField.setEnabled(selectNbestParser.isSelected());
		nbestParserBrowseButton.setEnabled(selectNbestParser.isSelected());
	    }
	    else if(object == selectDeleteFilter){
		deleteFilterField.setEnabled(selectDeleteFilter.isSelected());
		deleteFilterBrowseButton.setEnabled(selectDeleteFilter.isSelected());
	    }
	    else if(object == selectSynGetBin){
		synGetBinField.setEnabled(selectSynGetBin.isSelected());
		synGetBinBrowseButton.setEnabled(selectSynGetBin.isSelected());
	    }
	    else if(object == featBin){  
		featBinFile.setEnabled(featBin.isSelected());  
		featBinFileButton.setEnabled(featBin.isSelected());
	    }
	    else if(object == featLex){  
		featLexFile.setEnabled(featLex.isSelected()); 
		featLexFileButton.setEnabled(featLex.isSelected()); 
	    }
	    else if(object == featGram){ 
		featGramFile.setEnabled(featGram.isSelected()); 
		featGramFileButton.setEnabled(featGram.isSelected()); 
	    }
	    else if(object == featPreProc){ 
		featPreProc.setEnabled(featPreProc.isSelected()); 
		featProcBinButton.setEnabled(featPreProc.isSelected()); 
	    }
	    // browser button events
	    else if(object ==gramFileButton){  setSelectedFile(1); }
	    else if(object ==featBinFileButton){   setSelectedFile(2); }
	    else if(object ==featLexFileButton){  setSelectedFile(3); }
	    else if(object ==featGramFileButton){  setSelectedFile(4); }
	    else if(object ==featProcBinButton){  setSelectedFile(5); }
	    // ok & cancel buttons
	    else if(object ==parserOk){  setOptions();  }
	    else if(object ==parserCancel){
		setVisible(false);
	    }
	}
    }
    
    public boolean updateFileOption(JTextField textfield, String paramName, JCheckBox checkbox){
	String filename;
	
	if(checkbox.isSelected()){
	    // get the text
	    filename=textfield.getText();
	    // check to see if it is a valid file name
	    File f = new File(filename);
	    if(f.isFile()){
		// update the filename in the prefs.
		paramName = filename;
		return true;
	    }
	    else{
		// send error message
		Error err = new Error(this, 1, filename + " is an invalid file name. Please enter valid file name");
		err.showErrMsg();
		return false;
	    }
	}
	return true;
    }

    
    public void setOptions(){
	// update the file if the option is checked
	if(!updateFileOption(gramFile,grammarFileName,selectGrammar)){ return; }
	// Set The Feature Binary to be used
	if(!updateFileOption(featBinFile,featBinFileName,featBin)){ return; }
	// set the feature lexicon to be used
	if(!updateFileOption(featLexFile,featLexFileName,featLex)){ return; }
	// set the Feature Grammar
	if(!updateFileOption(featGramFile,featGramFileName,featGram)){ return; }
	if(!updateFileOption(featProcBin,featProcBinFileName,featPreProc)){ return; }
	setVisible(false);
	writeNewDefaults();
    }
    
    /*
      Insert code to send preferences to server thru port.
     */
    
    public void writeNewDefaults(){
	String token;
	String output = "# Do not edit this file directly to avoid \n" +
	    "# damaging the structure. \n" +
	    "# To modify file, load the Parser \n" +
	    "# Server and use the \"Configure Parser\" menu item.\n" + 
	    grammar_file + "=" + grammarFileName +"\n" +
	    parser_bin + "=" + parserBinFileName + "\n" +
	    nbestparser_bin + "=" + nbestParserBinFileName +"\n" +
	    synget_bin + "=" + synGetBinFileName +"\n" +
	    deletefilter_bin + "=" + deleteFilterBinFileName +"\n" +
	    checker + "=" + featBinFileName + "\n" + 
	    feat_postprocessor + "=" +featProcBinFileName+"\n"+
	    gramfeat_file + "=" +featGramFileName + "\n" +
	    lexfeat_file + "=" + featLexFileName + "\n" +
	    "end\n" ;
	// open file for writing
	try{
	    DataOutputStream outputStream = new DataOutputStream(new FileOutputStream(pathNamesFile));
	    outputStream.writeBytes(output);
	    outputStream.close();
	}
	catch(Exception e){  
	    Error err = new Error(this, 1,"Error: File could not be written" );
	}
	
    }
    public void setSelectedFile(int file){
	String selectedFile;
	JFileChooser chooser = new JFileChooser();
	String grammarFile,featureBinaryFile ,featureLexiconFile,featureGrammarFile,featurePreprocBinary;
	switch(file){
	case 1:
	    {
		chooser.setDialogTitle("Select Grammar File");
		chooser.showDialog(this, "Select");
		try{  
		    grammarFile =(chooser.getSelectedFile()).toString(); 
		    gramFile.setText( grammarFile);
		}
		catch(Exception e){ 
		    selectGrammar.setSelected(false);
		    gramFile.setEnabled(false); 
		    gramFileButton.setEnabled(false); 
		} 
		break;
	    }
	case 2:
	    {
		chooser.setDialogTitle("Select Feature Binary");
		chooser.showDialog(this, "select");
		try{  
		    featureBinaryFile=(chooser.getSelectedFile()).toString(); 
		    featBinFile.setText(featureBinaryFile);
		}
		catch(Exception e){
		    featBin.setSelected(false);
		    featBinFile.setEnabled(false);  
		    featBinFileButton.setEnabled(false);
		} 
		break;
	    }
	case 3:
	    {
		chooser.setDialogTitle("Select Feature Lexicon");
		chooser.showDialog(this, "select");
		try{  
		    featureLexiconFile=(chooser.getSelectedFile()).toString(); 
		    featLexFile.setText(featureLexiconFile);
		}
		catch(Exception e){
		    featLex.setSelected(false);
		    featLexFile.setEnabled(false); 
		    featLexFileButton.setEnabled(false); 
		} 
		break;
	    }
	case 4:
	    {
		chooser.setDialogTitle("Select Feature Grammar File");
		chooser.showDialog(this, "Select");
		try{  
		    featureGrammarFile=(chooser.getSelectedFile()).toString();
		    featGramFile.setText(featureGrammarFile);
		}
		catch(Exception e){
		    featGram.setSelected(false);
		    featGramFile.setEnabled(false); 
		    featGramFileButton.setEnabled(false); 
		} 
		break;
	    }
	case 5:
	    {
		chooser.setDialogTitle("Select Feature Preprocessor Binary");
		chooser.showDialog(this, "open");
		try{  
		    featurePreprocBinary=(chooser.getSelectedFile()).toString(); 
		    featPreProc.setText( featurePreprocBinary);
		}
		catch(Exception e){  
		    featPreProc.setSelected(false);
		    featProcBin.setEnabled(false); 
		    featProcBinButton.setEnabled(false); 
		}
		break;
	    }
	    
	}
	
    }    

    /*
      Name: 
          setPaths
      Args:
      Returns: void
      Description:
                 Receives a pathname to a file containing server preferences
		 Reads the files and set the value for the files
		 Sets the fields in the preferences window
		 
     */
    
    public void setPaths(){
	File file;
	FileInputStream fileStream;
	String line;
	BufferedReader input;
	int i;
	String value, param;
	// open the preferences filename
	file = new File(pathNamesFile);
	if(file.isFile()){
	    try{
		fileStream = new FileInputStream(pathNamesFile);
		// read the preferences
		input = new BufferedReader(new InputStreamReader(fileStream));
		// fill in the prefs array
		line = input.readLine();
		while(line != null && line.compareTo("end") != 0){
		    if(line.startsWith("#") || line.compareTo("")==0){} // ignore the line
		    else{
			// get the parameter
			param = line.substring(0, line.indexOf("="));
			value = line.substring(line.indexOf("=")+1);
			if(param.compareTo(grammar_file)==0){  grammarFileName = value; }
			else if(param.compareTo(parser_bin)==0){  parserBinFileName = value; }
			else if(param.compareTo(nbestparser_bin)==0){ nbestParserBinFileName = value; }
			else if(param.compareTo(synget_bin)==0){  synGetBinFileName=value;  }
			else if(param.compareTo(deletefilter_bin)==0){  deleteFilterBinFileName = value;  }
			else if(param.compareTo(checker)==0){   featBinFileName = value; }
			else if(param.compareTo(lexfeat_file)==0){  featLexFileName = value; }
			else if(param.compareTo(gramfeat_file)==0){ featGramFileName = value;  }
			else if(param.compareTo(feat_postprocessor)==0){  featProcBinFileName = value;  }
		    }
		    line=input.readLine();
		}
	    }
	    catch(Exception e){
		e.printStackTrace();
	    }
	}
    }
    
    public static void main(String args[]){
	// create a new JFrame
	JFrame test = new JFrame("Test Frame");
       	// create new dialog
	final ServerConfigWindow dialog = new ServerConfigWindow(test, "Parser Options");
	// associate dialog box
	JButton b = new JButton("Show Dialog");
	b.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent e) {
		    dialog.setVisible(true);
		}
	    });
	test.setSize(400,400);
	test.getContentPane().add(b);
	test.setVisible(true);
    }
    


}
