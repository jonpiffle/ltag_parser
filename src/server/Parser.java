import java.io.*;
import java.net.*;
import java.awt.*;
import java.util.*;
import javax.swing.event.*;
import javax.swing.*;
import java.awt.event.*;

public class Parser
{
    Hashtable preferences;
    String args;
        
    InputManager processStdOut=null;
    InputManager processStdErr=null;
    String stdOutDelimiter="StdOut Stream";
    String stdErrDelimiter="StdErr Stream";
    // parameter names:
    String lower_case="lower_case";
    String remove_punctuation="remove_punctuation";
    String run_tree_filter="run_tree_filter";
    String use_nbestparser_bin="use_nbestparser_bin";
     String check_feat="check_feat";
    String unify_features="unify_features";
    // parameter names for files
    String grammar_file="grammar_file";
    String parser_bin="parser_bin";
    String nbestparser_bin="nbestparser_bin";
    String synget_bin="synget_bin";
    String deletefilter_bin="deletefilter_bin";
    String checker="checker";
    String lexfeat_file="lexfeat_file";
    String gramfeat_file="gramfeat_file";
    String feat_postprocessor="feat_postprocessor";
    // file names
    String grammar, parser, nbestParser, synget, deletefilter, featChecker, lexfeat, gramfeat, feat_proc;
    // xtag.prefs
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
    String parser_max_start_count = "parser_max_start_count";
    String parser_max_parse_time_allowed="parser_max_parse_time_allowed";
    // preferences file
    String prefsFile;
    String pathNamesFile = ServerConfigWindow.pathNamesFile;
    // options
    boolean lowerCaseOpt;
    boolean removePunctOpt;
    boolean nbestParserOpt;
    boolean  deleteFilterOpt;
    boolean unifierOpt;
    DataOutputStream outputToServer;
    // processes
    MyProcess deleteFilterProcess,synGetBinProcess,parserProcess, nbestParserProcess,checkerProcess,featProcProcess;
    
    
    public Parser(Hashtable prefs, DataOutputStream output){
	preferences = prefs;
	outputToServer = output;
	setPaths();
    }
    
    public void setPrefs(Hashtable prefs){
	preferences = prefs;
	reconfigureParser();
    }

    public void setPaths(){
	File file;
	FileInputStream fileStream;
	String line;
	BufferedReader input;
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
			if(param.compareTo(grammar_file)==0){  grammar = value; }
			else if(param.compareTo(parser_bin)==0){  parser = value; }
			else if(param.compareTo(nbestparser_bin)==0){  nbestParser = value; }
			else if(param.compareTo(synget_bin)==0){  synget=value;  }
			else if(param.compareTo(deletefilter_bin)==0){  deletefilter = value;  }
			else if(param.compareTo(checker)==0){   featChecker = value; }
			else if(param.compareTo(lexfeat_file)==0){  lexfeat = value; }
			else if(param.compareTo(gramfeat_file)==0){ gramfeat = value;  }
			else if(param.compareTo(feat_postprocessor)==0){  feat_proc = value;  }
		    }
		    line=input.readLine();
		}
	    }
	    catch(Exception e){
		e.printStackTrace();
	    }
	}
    }

    
    public  MyProcess initProcess(String cmd[]){
	Process process=null;
	Runtime runtime = Runtime.getRuntime();
	BufferedReader stdOut, stdErr;
	OutputStream stdIn;
	MyProcess proc=null;
	try{
	    // start the parser
	    process = runtime.exec(cmd);
	    // create the parser streams
	    stdOut= new BufferedReader(new InputStreamReader(process.getInputStream()));
	    stdErr= new BufferedReader(new InputStreamReader(process.getErrorStream()));
	    // for the process, create a thread to listen to the stderr stream
	    (new InputManager(stdErr, outputToServer)).start();
	    stdIn = process.getOutputStream();
	    proc = new MyProcess( process,stdOut,stdErr,stdIn);
	}
	catch(Exception e){
	    e.printStackTrace();
	}
	return proc;
    }
    
    /*
      Name: 
           initParser
      Args:
          void
           
      Returns
          MyProcess Object containing parser process and associated data streams
      Description:
          Checks to see if the nbest parser is being used and sets the parser binary 
	  appropriately (nbestparser_bin or parser_bin). The grammar file and 
	  preferences file are then set. A new parser process is created and returned
      
     */
    
    public MyProcess initParser(){
	String cmd[] = new String[3]; 
	cmd[0]=parser; 
	// setup the command to be executes
	cmd[1]=grammar;
	cmd[2]=prefsFile;
	return initProcess(cmd);
    }
        
    
    /*
      Name: 
           initNbestParser
      Args:
          void
           
      Returns
          MyProcess Object containing parser process and associated data streams
      Description:
          initializes nbest parser
	
      
     */
    
    public MyProcess initNbestParser(){
	String cmd[] = new String[3]; 
	cmd[0]= nbestParser; 
	cmd[1]=grammar;
	cmd[2]=prefsFile;
	return initProcess(cmd);
    }


    /*
      Name:
          initChecker
      Args:
          void
      Returns:
          MyProcess Object: contains checker process and associated datastreams
      Description:
          Sets the command string, the lexicon feature file and grammar feature file
	  and creates a new process which is returned
      
     */

    public MyProcess initChecker(){
	String cmd[] = new String[3];
	cmd[0]=featChecker;
	cmd[1]=lexfeat;
	cmd[2]=gramfeat;
	return initProcess(cmd);
    }
    
    /*
      Name:
          initFeatProcessor
      Args:
          void
      Returns:
          MyProcess Object: contains feature post-processor process and associated datastreams
      Description:
          Sets the command string  and creates a new process which is returned
      
     */

    public MyProcess initFeatProcessor(){
	String cmd[] = new String[1];
	cmd[0]=feat_proc;
	return initProcess(cmd);
    }
    

    
    public MyProcess initSynGetBin(){
	String cmd[] = new String[3];
	cmd[0]= synget;
	cmd[1]=grammar;
	cmd[2]=prefsFile;
	return initProcess(cmd);
    }
    
    public MyProcess initDeleteFilter(){
	String cmd[] = new String[1];
	cmd[0]=deletefilter;
	return initProcess(cmd);
    }
        
    
    
    public int configParser(String prefsFile){
	this.prefsFile = prefsFile;
	// create the default preferences file
	if(writePrefsFile(this.prefsFile)==0){
	    System.out.println("Error: preferences file could not be written");
	    return 0;
	}
	// setup the options
	setOptions();
	if(deleteFilterOpt || nbestParserOpt){
	    deleteFilterProcess = initDeleteFilter();
	    synGetBinProcess=initSynGetBin();
	}
	if(nbestParserOpt){  nbestParserProcess = initNbestParser();  }
	else{  parserProcess = initParser(); }
	
	if(unifierOpt){
	    checkerProcess = initChecker();
	    featProcProcess=initFeatProcessor();
	}
	return 1;
    }

    
    public String parse(String input){
	int inputLength;
	String cstr;
	char c;
	int i;
	String str = new String(input);
	System.out.println("Server Received: " + str);
	// check to see if the final punct is to be removed
	if(removePunctOpt){
	    inputLength = str.length();
	    for(i=inputLength-1; i >= 0 ; i--){
		c = str.charAt(i);
		if(c=='.' || c=='?' || c=='!'){ 
		    str = str.substring(0, i) + "\n";
		    
		    break;
		}
		else if(c==' ' || c=='\n' || c=='\t'){}
		else{ break; }
	    }
	}
	// check if the first letter in the sentence is to made lower case
	if(lowerCaseOpt){
	    inputLength = str.length();
	    for(i=0; i < inputLength ; i++){
		c = str.charAt(i);
		if(c==' ' || c=='\n' || c=='\t'){}
		else{ 
		    cstr = c + "";
		    cstr = cstr.toLowerCase();
		    str = cstr + str.substring(i+1);
		    break; 
		}
	    }
	}
	// if nbestparser is being used
	if(nbestParserOpt){
	    // pass the input thru the syngetbin, then thru deletefilterbin
	    str=pipeToProcess(synGetBinProcess, str, false);
	    str= pipeToProcess(deleteFilterProcess,str, false);
	    // parser result using nbestParser
	    str=pipeToProcess(nbestParserProcess,str,false);
	}
	else{
	    // parser result using parser
	    str=pipeToProcess(parserProcess,str,false);
	    // ckeck if flag for checking features is set
	    if(unifierOpt){
		str=pipeToProcess(checkerProcess, str,false);
		str= pipeToProcess(featProcProcess, str, false);
	    }
	}
	System.out.println("returning output for: " + input+"\n");
	return str;
    }


    
    public String pipeToProcess(MyProcess myprocess, String input, boolean debug){
	String result=null;
	try{
	    // create new threads to listen to read the output streams from the parser process
	    processStdOut = new InputManager(myprocess.stdOut, debug);
	    //processStdErr = new InputManager(myprocess.stdErr, stdErrDelimiter); 
	    // start the streams
	    processStdOut.start();
	    //processStdErr.start();
	    // write to the process
	    myprocess.stdIn.write((input).getBytes());
	    myprocess.stdIn.flush();
	    // wait for the threads to die
	    //processStdErr.join();
	    processStdOut.join();
	    result=processStdOut.output;
	}
	catch(Exception e){
	    e.printStackTrace();
	}
	return result;
    }
    
    
    public void reconfigureParser(){
	setOptions();
	killAll();
	configParser(prefsFile);

    }
    
    public int writePrefsFile(String filename){
	File f;
	String output="";
	try{
	    DataOutputStream outputStream = new DataOutputStream(new FileOutputStream(filename));
	    output= print_all_prefs +"="+(String)preferences.get(print_all_prefs)+"\n"+
		buffer_stdout + "=" +(String)preferences.get(buffer_stdout)+"\n"+
		err_quiet +"="+(String)preferences.get(err_quiet)+"\n"+ 
		prune_first_pass+"="+(String)preferences.get(prune_first_pass)+"\n"+ 
		prune_syn_lookup+"="+(String)preferences.get(prune_syn_lookup)+"\n"+
		anchors_match_syn_subscript+"="+(String)preferences.get(anchors_match_syn_subscript)+"\n"+ 
		warn_when_pruning+"="+(String)preferences.get(warn_when_pruning)+"\n"+ 
		syn_xtag_treenames+"="+(String)preferences.get(syn_xtag_treenames)+"\n"+ 
		allow_foot_adjoin+"="+(String)preferences.get(allow_foot_adjoin)+"\n"+ 
		allow_anchor_adjoin+"="+(String)preferences.get(allow_anchor_adjoin)+"\n"+ 
		allow_eps_trees+"="+(String)preferences.get(allow_eps_trees)+"\n"+ 
		parser_print_treenodes+"="+(String)preferences.get(parser_print_treenodes)+"\n"+ 
		parser_print_feats+"="+(String)preferences.get(parser_print_feats)+"\n"+ 
		parser_print_spans+"="+(String)preferences.get(parser_print_spans)+"\n"+ 
		parser_allow_eps_adjoin+"="+(String)preferences.get(parser_allow_eps_adjoin)+"\n"+ 
		parser_disallow_leftaux_wrapping +"="+(String)preferences.get(parser_disallow_leftaux_wrapping)+"\n"+
		parser_disallow_rightaux_wrapping +"="+(String)preferences.get(parser_disallow_rightaux_wrapping)+"\n"+ 
		parser_multiple_adjunctions +"="+(String)preferences.get(parser_multiple_adjunctions)+"\n"+ 
		parser_max_start_count + "=" + "0" + "\n" +
		parser_max_parse_time_allowed + "=" + "240.0" + "\n";
	    outputStream.writeBytes(output);
	    outputStream.close();
	    return 1;
	}
	catch(Exception e){
	    return 0;
	}
	
	
    }
    
    
    public void setOptions(){
	// lower case
	if(((String)preferences.get(lower_case)).compareTo("1")==0){          lowerCaseOpt=true; }
	else{ lowerCaseOpt=false ;  } 
	// remove final punctuation marks
	if(((String)preferences.get(remove_punctuation)).compareTo("1")==0){  removePunctOpt=true; }
	else{  removePunctOpt=false;}
	// filter tree option
	//if(((String)preferences.get(run_tree_filter)).compareTo("1")==0){      deleteFilterOpt=true; }
	//else{   deleteFilterOpt=false;  }
	// use nbestparser
	if(((String)preferences.get(use_nbestparser_bin)).compareTo("1")==0){       nbestParserOpt=true;  deleteFilterOpt=true;  }
	else{  nbestParserOpt=false; }
	// unifier option
	if(((String)preferences.get(unify_features)).compareTo("1")==0){        unifierOpt=true; }
	else{  unifierOpt=false;  }
    }
    

    /*
      Used only when exiting or after preferences have changed. Cannot be used during a parse
    */
    public void killAll(){
	if(deleteFilterProcess !=null){  deleteFilterProcess.process.destroy(); }
	if(synGetBinProcess != null){    synGetBinProcess.process.destroy();    }
	if(parserProcess != null){       parserProcess.process.destroy();       }
	if(checkerProcess!= null ){      checkerProcess.process.destroy();      }
	if(featProcProcess != null){     featProcProcess.process.destroy();     }
    }

    
    public void finish(){
	// allows calling process to wait until all threads are done
    }
    
}


