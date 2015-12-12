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
      ParserConfigWindow
  Description
      Creates a user interface to allow user to configure server
      
 */


public class ParserConfigWindow extends JDialog
{
    ShowTrees currentShowTrees;
    Container container;
    JPanel parserOpts, grammar, features, confirm, otherOptions;
    JTabbedPane tabbedPane;
    // checkbox items for configuring components of parser to be used in parsing
    JCheckBox runTreeFilter, checkFeat, lowerCase, remPunct,unifyFeat, useNbestParser;
    // checkbox items for activating file setting textboxes
    JCheckBox useGram,  featBin, featLex, featGram, featPreProc;
    // textfields for entering filenames for different files used by parser
    JTextField gramFile,featBinFile,featLexFile,featGramFile,featProcBin;
    // button for browsing files when setting paths for files used by parser
    JButton gramFileButton, featBinFileButton,featLexFileButton,featGramFileButton,featProcBinButton;
    // checkboxes for other options 
    JCheckBox supressErrors, pruneFirstPass, pruneSynLookup,anchorsMatchSubScript, warnWhenPruning, synXtagTreeNames;
    JCheckBox allowFootAdjoin, allowAnchorAdjoin, allowEpsTrees, printTreeNodes, printFeats, printSpans;
    JCheckBox allowEpsAdjoin,disallowLeftAuxWrapping, disallowRightAuxWrapping, multipleAdjunctions, maxStartCount, maxParseTime;
    // buttons for dismissing configuration window
    JButton parserOk, parserCancel;
    // default path to preferences
    //String preferencesFile="E:\\browser\\data\\server.prefs";
    String preferencesFile = "/mnt/linc/xtag/work/moses/browser/data/server.prefs";
    // Parameters for configuration Options for Running the Parser
    String unify_features="unify_features";
    String lower_case="lower_case";
    String remove_punctuation="remove_punctuation";
    String run_tree_filter="run_tree_filter";
    String use_nbestparser_bin="use_nbestparser_bin";
    String check_feat="check_feat";
    
    // Parameters for Parser Binary
    String print_all_prefs="print_all_prefs";
    String buffer_stdout="buffer_stdout";
    String err_quiet="err_quiet";
    String prune_first_pass="prune_first_pass";
    String prune_syn_lookup="prune_syn_lookup";
    String anchors_match_syn_subscript="anchors_match_syn_subscript";
    String warn_when_pruning="warn_when_pruning";
    String syn_xtag_treenames="syn_xtag_treenames";
    String allow_foot_adjoin="allow_foot_adjoin";
    String allow_anchor_adjoin="allow_anchor_adjoin";
    String allow_eps_trees ="allow_eps_trees";
    String parser_print_treenodes="parser_print_treenodes";
    String parser_print_feats="parser_print_feats";
    String parser_print_spans="parser_print_spans";
    String parser_allow_eps_adjoin="parser_allow_eps_adjoin";
    String parser_disallow_leftaux_wrapping="parser_disallow_leftaux_wrapping";
    String parser_disallow_rightaux_wrapping="parser_disallow_rightaux_wrapping";
    String parser_multiple_adjunctions="parser_multiple_adjunctions";
    // Labels
    String lowerCaseLab, remPunctLab,runTreeFilterLab,checkFeatLab,nbestParserLab;
    // grammar labels
    String useGramLab;
    // features labels
    String unifyFeatLab,featBinLab, featLexLab,featGramLab, featPreProcLab;
    // tab descriptions
    String punctDesc, gramDesc,featDesc, browse;
    
    // Hashtable for storing preferences
    Hashtable prefs;
    public ParserConfigWindow(ShowTrees frame, String title){
	super(frame, title, true);
	currentShowTrees = frame;
	FontMetrics fm ;
	int height;
	int cols=30;
	prefs = new Hashtable(40);
	// Labels
	lowerCaseLab = " lowercase first letter of first word in sentence";
	remPunctLab = " remove sentence final punctuation";
	unifyFeatLab = " unify features" ;
	runTreeFilterLab="Run Parser with Tree Filter";
	checkFeatLab = "Check Features";
	nbestParserLab="Use Nbest Parser";
	// tab descriptions
	punctDesc = "How to handle punctuation";
	featDesc = "Configure Feature handling";
	// create a listener for the bottons etc
	ParserActionListener listener = new ParserActionListener();
	browse = "Browse";
	container = getContentPane();
	container.setLayout(new BorderLayout());
	// create panels for each of the preference groups
	parserOpts= new JPanel(new GridLayout(4,1));
	grammar = new JPanel(null);
	features  = new JPanel(null);
	confirm = new JPanel();
	otherOptions = new JPanel(new GridLayout(16,1));
	
	// set the default values
	setDefaults();
	fm= getFontMetrics(container.getFont());
	height  = fm.getHeight()+5;
	
	//  ********************* Panel For Parse Options  ***************
	lowerCase   = new JCheckBox(lowerCaseLab, ((String)prefs.get(lower_case)).startsWith("1"));
	remPunct    = new JCheckBox(remPunctLab, ((String)prefs.get(remove_punctuation)).startsWith("1"));
	unifyFeat = new JCheckBox(unifyFeatLab, ((String)prefs.get(unify_features)).startsWith("1"));
	runTreeFilter = new JCheckBox(runTreeFilterLab, ((String)prefs.get(run_tree_filter)).startsWith("1"));
	useNbestParser=new JCheckBox(nbestParserLab, ((String)prefs.get(use_nbestparser_bin)).startsWith("1"));
	checkFeat=new JCheckBox(checkFeatLab, ((String)prefs.get(check_feat)).startsWith("1"));
	
	parserOpts.add(lowerCase);
	parserOpts.add(remPunct);
	parserOpts.add(unifyFeat);
	//parserOpts.add(runTreeFilter);
	parserOpts.add(useNbestParser);
	//parserOpts.add(checkFeat);

	
	// *********** Other OPtions Panel  *************
	supressErrors=new JCheckBox("Suppress Errors", ((String)prefs.get(err_quiet)).startsWith("1"));
	pruneFirstPass=new JCheckBox("Prune on first pass", ((String)prefs.get(prune_first_pass)).startsWith("1"));
	pruneSynLookup=new JCheckBox("Prune with Syn Lookup", ((String)prefs.get(prune_syn_lookup)).startsWith("1"));
	anchorsMatchSubScript=new JCheckBox("Anchor Match as Syn Subscript",
					    ((String)prefs.get(anchors_match_syn_subscript)).startsWith("1") );
	warnWhenPruning=new JCheckBox("Warn when pruning", ((String)prefs.get(warn_when_pruning)).startsWith("1")); 
	synXtagTreeNames=new JCheckBox("Syn Xtag TreeNames", ((String)prefs.get(syn_xtag_treenames)).startsWith("1"));
	allowFootAdjoin=new JCheckBox(" Allow Foot to adjoin",((String)prefs.get(allow_foot_adjoin)).startsWith("1") );
	allowAnchorAdjoin=new JCheckBox("Allow anchor to adjoin", 
					((String)prefs.get(allow_anchor_adjoin)).startsWith("1"));
	allowEpsTrees=new JCheckBox("allow eps trees",((String)prefs.get(allow_eps_trees)).startsWith("1") );
	printTreeNodes = new JCheckBox("print tree nodes", ((String)prefs.get(parser_print_treenodes)).startsWith("1"));
	printFeats= new JCheckBox("print features", ((String)prefs.get(parser_print_feats)).startsWith("1"));
	printSpans= new JCheckBox("print spans",((String)prefs.get(parser_print_spans)).startsWith("1"));
	allowEpsAdjoin = new JCheckBox("allow EPS adjoin",
				       ((String)prefs.get(parser_allow_eps_adjoin)).startsWith("1") );
	disallowLeftAuxWrapping=new JCheckBox("Disallow Leftaux Wrapping",
					      ((String)prefs.get(parser_disallow_leftaux_wrapping)).startsWith("1") ); 
	disallowRightAuxWrapping=new JCheckBox("Disallow rigthaux wrapping", 
					       ((String)prefs.get(parser_disallow_rightaux_wrapping)).startsWith("1"));
	multipleAdjunctions=new JCheckBox("Multiple adjunctions",
					  ((String)prefs.get(parser_multiple_adjunctions)).startsWith("1") );
	//maxStartCount=new JCheckBox();
	//maxParseTime=new JCheckBox();
	
	// set the layout 
	otherOptions.setLayout(new GridLayout(16,1));
	
	// add the checkboxes to the panel
	otherOptions.add(supressErrors);
	otherOptions.add(pruneFirstPass);
	otherOptions.add(pruneSynLookup);
	otherOptions.add(anchorsMatchSubScript);
	otherOptions.add(warnWhenPruning);
	otherOptions.add(synXtagTreeNames);
	otherOptions.add(allowFootAdjoin);
	otherOptions.add(allowAnchorAdjoin);
	otherOptions.add(allowEpsTrees);
	otherOptions.add(printTreeNodes);
	otherOptions.add(printFeats);
	otherOptions.add(printSpans);
	otherOptions.add(allowEpsAdjoin);
	otherOptions.add(disallowLeftAuxWrapping);
	otherOptions.add(disallowRightAuxWrapping);
	otherOptions.add(multipleAdjunctions);
	

	// create a tabbed pane to contain punctuation, grammar, features and confirm panels
	tabbedPane = new JTabbedPane();
	//tabbedPane.setSize(600,450);
	tabbedPane.addTab("Parser", null, parserOpts, punctDesc); 
	//tabbedPane.addTab("Grammar", null, grammar,gramDesc); 
	//tabbedPane.addTab("Features", null, features,featDesc); 
	tabbedPane.addTab("Other Options", null, otherOptions, null);
	// add the tabbed pane 
	tabbedPane.setSize(600, 450);
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
	    // ok & cancel buttons
	    if(object ==parserOk){  setOptions();  }
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
		prefs.remove(paramName);
		prefs.put(paramName, filename);
		return true;
	    }
	    else{
		// send error message
		Error err = new Error(this, 1, filename + " is an invalid file name. Please enter valid file name");
		err.showErrMsg();
		return false;
	    }
	}
	else{
	    filename = (String)prefs.get(paramName);
	    prefs.remove(paramName);
	    prefs.put(paramName, filename);
	    return true;
	}
    }

    public void updateBooleanOption(JCheckBox checkbox, String param){
	prefs.remove(param);
	if(checkbox.isSelected()){ prefs.put(param, "1"); }
	else{ prefs.put(param, "0"); }
    }
    
    public void setOptions(){
	// update configuration of parser
	updateBooleanOption(unifyFeat,unify_features);
	updateBooleanOption(lowerCase,lower_case);
	updateBooleanOption(remPunct,remove_punctuation);
	updateBooleanOption(runTreeFilter,run_tree_filter);
	updateBooleanOption(checkFeat,check_feat);
	updateBooleanOption(useNbestParser,use_nbestparser_bin);
	// update xtag.prefs
	updateBooleanOption(supressErrors,err_quiet);
	updateBooleanOption(pruneFirstPass,prune_first_pass);
	updateBooleanOption(pruneSynLookup,prune_syn_lookup);
	updateBooleanOption(anchorsMatchSubScript,anchors_match_syn_subscript);
	updateBooleanOption(warnWhenPruning,warn_when_pruning);
	updateBooleanOption(synXtagTreeNames,syn_xtag_treenames);
	updateBooleanOption(allowFootAdjoin,allow_foot_adjoin);
	updateBooleanOption(allowAnchorAdjoin,allow_foot_adjoin);
	updateBooleanOption(allowEpsTrees,allow_eps_trees);
	updateBooleanOption(printTreeNodes,parser_print_treenodes);
	updateBooleanOption(printFeats,parser_print_feats);
	updateBooleanOption(printSpans,parser_print_spans);
	updateBooleanOption(allowEpsAdjoin,parser_allow_eps_adjoin);
	updateBooleanOption(disallowLeftAuxWrapping,parser_disallow_leftaux_wrapping);
	updateBooleanOption(disallowRightAuxWrapping,parser_disallow_rightaux_wrapping);
	updateBooleanOption(multipleAdjunctions,parser_multiple_adjunctions);
	setVisible(false);
	writeNewDefaults();
	if(currentShowTrees.clientWindow != null){
	    if(currentShowTrees.clientWindow.client != null){
		currentShowTrees.clientWindow.client.sendPreferences(preferencesFile);
	    }
	}
    }
    
    /*
      Insert code to send preferences to server thru port.
     */
    
    public void writeNewDefaults(){
	String hashContents=prefs.toString();
	String token;
	String output = "# Do not edit this file directly to avoid \n" +
	    "# damaging the structure. \n" +
	    "# To modify file, load the Parser \n" +
	    "# Server and use the \"Configure Parser\" menu item.\n";
	hashContents=hashContents.substring(hashContents.indexOf("{")+1, hashContents.indexOf("}"));
	StringTokenizer tokenizer = new StringTokenizer(hashContents, ", ");
	int i = tokenizer.countTokens();
	int j=0;
	while(j < i){
	    token = tokenizer.nextToken();
	    token=token+"\n";
	    output = output+token;
	    j++;
	}
	output=output+"end\n";
	// open file for writing
	try{
	    DataOutputStream outputStream = new DataOutputStream(new FileOutputStream(preferencesFile));
	    outputStream.writeBytes(output);
	    outputStream.close();
	}
	catch(Exception e){  
	    Error err = new Error(this, 1,"Error: File could not be written" );
	}
	
    }
    
    /*
      Name: 
          setDefaults
      Args:
      Returns: prefs: Array of pathnames specified in the preferences
      Description:
                 Receives a pathname to a file containing server preferences
		 Reads the files and set the value for the files
		 Sets the fields in the preferences window
		 returns a copy of the preferences to the calling function

     */
    
    public Hashtable setDefaults(){
	File file;
	FileInputStream fileStream;
	String line;
	BufferedReader input;
	int i;
	String value, param;
	// open the preferences filename
	file = new File(preferencesFile);
	if(file.isFile()){
	    try{
		fileStream = new FileInputStream(preferencesFile);
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
			prefs.put(param, value);
		    }
		    line=input.readLine();
		}
	    }
	    catch(Exception e){
		e.printStackTrace();
	    }
	    
	}
	else{
	    System.out.println("Preferences file " + preferencesFile + " could notbe found");
	}
	return prefs;
    }

}
