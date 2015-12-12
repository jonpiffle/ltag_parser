import java.io.*;
import java.net.*;
import java.awt.*;
import java.util.*;
import javax.swing.event.*;
import javax.swing.*;
import java.awt.event.*;


public class ClientWindow extends JFrame{
    JTextArea clientDisplay; // used by client to display client activity
    JTextArea serverDisplay; // used by client to display server messages
    JTextArea inputDisplay;  // used by user to enter new input sentences
    
    JScrollPane serverScrollPane;
    JScrollPane clientScrollPane;
    JScrollPane inputScrollPane;
    
    JSplitPane serverAndClient; // horizontally split pane, left half-> server, right half-> client
    JSplitPane inputServerAndClient; // vertically split pane: top half-> input area, bottom half-> server and client
    JToolBar toolBar;
    JButton connect;
    JButton disconnect;
    JButton loadFile;
    JButton saveGraph;
    JButton parseInput;
    JButton clearInput;
    JButton closeParser;
    String prefsFileName = "/mnt/linc/xtag/work/moses/browser/data/server.prefs";
    //String prefsFileName = "E:\\browser\\data\\server.prefs";
    String tempOutputFileName;
    String previousInputBuffer="";
    FileOutputStream tempOutputFileStream;
    boolean connected, graphPresent;
    boolean serverLoaded=false;
    boolean busy;
    ShowTrees currentShowTrees;
    Client client;
    String hostName;
    int portNumber;
    public ClientWindow(ShowTrees st){
	super("Client");
	currentShowTrees=st;
    }

    public int initClientWindow(){
	hostName = "caelum.cis.upenn.edu";
	portNumber = 1097;
	tempOutputFileName = "/tmp/temp_graph" + System.currentTimeMillis();
	//tempOutputFileName = "E:\\browser\\tmp_graphs\\temp_graph" + System.currentTimeMillis();
	toolBar = new JToolBar();
	// connect to server
	connect = new JButton("Connect");
	connect.setToolTipText("connect to server");
	toolBar.add(connect);
	// disconnect from server
	disconnect = new JButton("Disconnect");
	disconnect.setToolTipText("disconnect from server");
	toolBar.add(disconnect);
	disconnect.setEnabled(false);
	// load file
	loadFile= new JButton("Open File");
	loadFile.setToolTipText("Open file");
        toolBar.add(loadFile);
	// save the graph 
	saveGraph = new JButton("Save Graph");
	saveGraph.setToolTipText("Save graph");
	saveGraph.setEnabled(false);
	toolBar.add(saveGraph);
	
	//parse input
	parseInput = new JButton("Parse");
	parseInput.setToolTipText("Parse current input");
	parseInput.setEnabled(false);
	toolBar.add(parseInput);
	//clear input
	clearInput = new JButton("Clear");
	clearInput.setToolTipText("Clear current input");
        toolBar.add(clearInput);
	// close parser
	closeParser = new JButton("Close");
	closeParser.setToolTipText("Close Parser");
	toolBar.add(closeParser);
	// set the actionlistener
	ClientButtonListener buttonListener = new ClientButtonListener();
	connect.addActionListener(buttonListener);
	disconnect.addActionListener(buttonListener);
	loadFile.addActionListener(buttonListener);
	saveGraph.addActionListener(buttonListener);
	parseInput.addActionListener(buttonListener);
	clearInput.addActionListener(buttonListener);
	closeParser.addActionListener(buttonListener);
	
	Container contentPane = getContentPane();
	connected=false;
	graphPresent=false;
	// create the display areas
	clientDisplay = new JTextArea();
	serverDisplay = new JTextArea();
	inputDisplay = new JTextArea();
	// put the text areas in a scrollpanes
	serverScrollPane = new JScrollPane(serverDisplay);
	clientScrollPane = new JScrollPane(clientDisplay);
	inputScrollPane = new JScrollPane(inputDisplay);
	// set the preferred and minimum sizes
	serverScrollPane.setPreferredSize(new Dimension(250, 145));
	serverScrollPane.setMinimumSize(new Dimension(30, 30));
	clientScrollPane.setPreferredSize(new Dimension(250, 145));
	clientScrollPane.setMinimumSize(new Dimension(30, 30));
	inputScrollPane.setPreferredSize(new Dimension(500, 300));
	inputScrollPane.setMinimumSize(new Dimension(60, 10));
	
	// set the borders
	serverScrollPane.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createCompoundBorder(BorderFactory.createTitledBorder("Server"),
													 BorderFactory.createEmptyBorder(5,5,5,5)), 
								      serverScrollPane.getBorder()));
	clientScrollPane.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createCompoundBorder(BorderFactory.createTitledBorder("Client"),
													 BorderFactory.createEmptyBorder(5,5,5,5)), 
								      clientScrollPane.getBorder()));
	inputScrollPane.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createCompoundBorder(BorderFactory.createTitledBorder("Input Window"),
													BorderFactory.createEmptyBorder(5,5,5,5)), 
								     inputScrollPane.getBorder()));
	
	serverScrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
	clientScrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
	inputScrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
	
	clientDisplay.setLineWrap(true);
	serverDisplay.setLineWrap(true);
	inputDisplay.setLineWrap(true);
	
	clientDisplay.setWrapStyleWord(true);
	serverDisplay.setWrapStyleWord(true);
	inputDisplay.setWrapStyleWord(true);
	
	// create the split pane to accomodate server and client
	serverAndClient = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,serverScrollPane,clientScrollPane);
	inputServerAndClient = new JSplitPane(JSplitPane.VERTICAL_SPLIT,inputScrollPane,serverAndClient);
	serverAndClient.setOneTouchExpandable(true);
	inputServerAndClient.setOneTouchExpandable(true);
	contentPane.add(toolBar,BorderLayout.NORTH);
	contentPane.add(inputServerAndClient,BorderLayout.CENTER);
	setBounds(300,0,600,800);
	return 1;
    }
    
    
    
    /*
      Name:
          parse
      Args:
          String input: sentence or sentences separated by newline character
	  
      Returns:
          void
      Description:
          Breaks up the input into individual sentences and sends one at a time to server and
	  waits for parsed output
	  
      Note:
          Assumes that the client has already been initialized.
     */

    public boolean parse(String input){
	String serverOutput[], token;
	
	DataOutputStream tempOutputDataStream=null;
	
	StringTokenizer tokenizer = new StringTokenizer(input, "\n");
	int numTokens = tokenizer.countTokens();
	boolean terminate=false;
	int j=0;
	  try{
		tempOutputFileStream = new FileOutputStream(tempOutputFileName);
		tempOutputDataStream= new DataOutputStream(tempOutputFileStream);
		// send one sentence at a time and receive the output from the server
		for(j=0; j < numTokens ; j++){
		    token=tokenizer.nextToken();
		    clientDisplay.append("parsing: " + token + "\n");
		    serverOutput = client.sendAndReceive(token+"\n", serverDisplay);
		    clientDisplay.append("done\n");
		    
		    if(serverOutput[0]!=null){
			// write the output to a file
			tempOutputDataStream.writeBytes(serverOutput[0]); 
			// enable save button
			saveGraph.setEnabled(true);
		    }
		    if(serverOutput[1].compareTo(">>TERMINATE<<") == 0){
			terminate=true;
			break;
		    }
		}
		// check to see if termination warning has been sent
		if(tempOutputDataStream !=null){   tempOutputDataStream.close(); }
	  }
	  catch(Exception e){
	      e.printStackTrace();
	  }
	  if(terminate){
	      clientDisplay.append("Server is shutting down. Cannot parse:\n");
	      for(int k= j; k < numTokens ; k++){
		  clientDisplay.append(tokenizer.nextToken()+"\n");
	      }
	  }
	  return terminate;
    }

    


    class ClientButtonListener implements java.awt.event.ActionListener
    {
	public void actionPerformed(java.awt.event.ActionEvent event)
	{
	    Object object = event.getSource();
	    if(object == connect){  
		if(connectToServer(portNumber, hostName)!=1){
		    System.out.println("could not connect to server");
		}
		// enable all the other buttons
		else{
		    disconnect.setEnabled(true);
		    parseInput.setEnabled(true);
		}
	    }
	    else if(object == disconnect){ 
		if(disconnectFromServer() !=1){
		    System.out.println("error occurred when closing connection");
		}
		else{
		    disconnect.setEnabled(false);
		    parseInput.setEnabled(false);
		}
	    }
	    else if (object == loadFile){ loadFileActionHandler(); }
	    else if(object == saveGraph){  saveGraphActionHandler();  }
	    else if(object == parseInput){ parseInputActionHandler(event); }
	    else if(object == clearInput){ clearInputActionHandler(event); }
	    else if(object == closeParser){ closeParserActionHandler();           }
	}
    }

    
    /*
      Name: 
          connectToServer
      Args:
          void
      Returns:
          int: 0=> failed to establish connection
	       1=> connection openned successfully
      Description:
          creates a new client and opens connection. Return the client if successful
	  otherwise returns null
      
     */
    
    public int  connectToServer(int portNumber,  String hostName){
	Client c= new Client(portNumber, hostName);
	if(c.openConnection()==1){  
	    client = c; 
	    // send the preferences
	    client.sendPreferences(prefsFileName);
	    return 1;
	}
	else{ 
	    client = null;
	    return 0;
	}
    }

    

    public int  disconnectFromServer(){
	int status=1;
	if(client != null){
	    status = client.closeConnection();
	    previousInputBuffer="";
	    client=null;
	}
	return status;
    }

    public void loadFileActionHandler(){
	File checkFile;
	String file;
	JFileChooser fileSaver;
	BufferedReader inputFileBufferedReader;
	String line;
	int promptResult;
	int status=1;
	// if there is a graph currently loaded, prompt user to save it
	if(graphPresent){
	    // save file ? yes:->call saveFileActionHandler(); no:-> delete the file
	    promptResult = JOptionPane.showConfirmDialog(this,
							 "Would you like to save current graph ?",
							 "",
							 JOptionPane.YES_NO_OPTION,
							 JOptionPane.INFORMATION_MESSAGE);
	    if(promptResult==JOptionPane.YES_OPTION){
		status=saveGraphActionHandler();
	    }
	    else if (promptResult == JOptionPane.NO_OPTION) {
		// delete the current tempFile
	    }
	}
	if(status==1){ // nothing to save or save completed successfully
	    //try{
		fileSaver = new JFileChooser();
		fileSaver.setDialogTitle("Input File");
		fileSaver.setApproveButtonText("Open");
		int userChoice = fileSaver.showDialog(this, "Open" );
		if(userChoice == JFileChooser.APPROVE_OPTION){  file=(fileSaver.getSelectedFile()).toString();  }
		else{  return;  }
	    file=(fileSaver.getSelectedFile()).toString();
		checkFile = new File(file);
		if(checkFile.isFile()){
		    try{ 
			// read the file and add the contents to inputDisplay
			inputFileBufferedReader = new BufferedReader( new InputStreamReader( new FileInputStream(file)));
			// read the lines one by one and add them to the display
			line = inputFileBufferedReader.readLine();
			while(line !=null){
			    inputDisplay.append(line + "\n");
			    line = inputFileBufferedReader.readLine();
			}
		    }
		    catch(Exception e){  
			JOptionPane.showMessageDialog(this,"Error occurred while attempting to read \""+ file + "\"","",JOptionPane.ERROR_MESSAGE);
		    }
		}
		else{
		    JOptionPane.showMessageDialog(this,"Error: \""+ file + "\" could not be found","",JOptionPane.ERROR_MESSAGE);
		    loadFileActionHandler();
		}
	    //}
	    //catch(Exception e){
		
	    //}
	}
	//otherwise, abort loading new file
    }
    
    public int  saveGraphActionHandler(){
	File file;
	DataOutputStream outputStream;
	JFileChooser fileSaver = new JFileChooser();
	String fileName;
	int promptResult;
	boolean writeFile=false;
	String line, errorMsg;
	boolean status;
	if(tempOutputFileStream!= null){
	    fileSaver.setDialogTitle("Save graph");
	    fileSaver.showSaveDialog(this);
	    try{
		// check if the file already exists
		fileName =(fileSaver.getSelectedFile()).toString();
		file = new File(fileName);
		// prompt to overwrite
		if(file.isFile()){ 
		    errorMsg= "\""+fileName+"\" exists. Would you like to overwrite this file ?";
		    promptResult = JOptionPane.showConfirmDialog(this,errorMsg,"", JOptionPane.YES_NO_OPTION,JOptionPane.INFORMATION_MESSAGE);
		    if(promptResult==JOptionPane.YES_OPTION){   file.delete(); }
		    else if (promptResult == JOptionPane.NO_OPTION){  return saveGraphActionHandler(); }
		    else if (promptResult == JOptionPane.CLOSED_OPTION){ return -1; } // -2 indicates that user did not 
		}
		try{
		    DataOutputStream savedFileStream = new DataOutputStream(new FileOutputStream(fileName));
		    FileInputStream f = new FileInputStream(tempOutputFileName);
		    BufferedReader input = new BufferedReader(new InputStreamReader(f));
		    //read from input stream and write to output stream
		    line = input.readLine();
		    while(line !=null){
			savedFileStream.writeBytes(line+"\n");
			line=input.readLine();
		    }
		    savedFileStream.close();
		    // disable the save button once you are done saving
		    saveGraph.setEnabled(false);
		}
		catch(Exception e){
		    errorMsg="Error: Cannot write output file " + fileName;
		    JOptionPane.showMessageDialog(this,errorMsg,"",JOptionPane.INFORMATION_MESSAGE);
		}
	    }
	    catch(Exception e){   return 0;  }// user has chosen to cancel save operation
	}
	// print message indicating that there is no graph to save
	else{
	    errorMsg="No graph to save.";
	    JOptionPane.showMessageDialog(this,errorMsg,"",JOptionPane.INFORMATION_MESSAGE);
	}
	graphPresent=false;
	return 1;
    }
    
    public void parseInputActionHandler(ActionEvent event){
	String newInputBuffer = inputDisplay.getText();
	File temp;
	// if the input is not an empty string, parse the input
	boolean terminate=false;
	busy = true;
	if(newInputBuffer.compareTo(previousInputBuffer) !=0 && newInputBuffer.compareTo("") !=0 ){
	    // if a temporary file exists, delete it
	    temp = new File(tempOutputFileName);
	    if(temp.exists()){  temp.delete(); }
	    else{  temp=null; }
	    setCursor(new Cursor(Cursor.WAIT_CURSOR));
	    terminate=parse(newInputBuffer);
	    graphPresent=true;
	    previousInputBuffer=newInputBuffer;
	    // call show trees to show graph
	    currentShowTrees.filename=tempOutputFileName;
	    currentShowTrees.openEventHandler(1);
	    setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
	}
	if(terminate){

	}
	busy = false;
    }
    
    public void clearInputActionHandler(ActionEvent event){
	inputDisplay.setText("");
	previousInputBuffer = "";
    }
    public void closeParserActionHandler(){
	int promptResult;
	int status=1;
	String errorMsg;
	File temp;
	
	// if there is a graph currently loaded, prompt user to save it
	if(graphPresent){
	    errorMsg= "Would you like to save current graph ?";
	    promptResult = JOptionPane.showConfirmDialog(this,errorMsg,"",JOptionPane.YES_NO_OPTION,JOptionPane.INFORMATION_MESSAGE);
	    if(promptResult==JOptionPane.YES_OPTION){ status = saveGraphActionHandler(); }
	    else if (promptResult == JOptionPane.NO_OPTION) {		
		// if a temporary file exists
		temp = new File(tempOutputFileName);
		if(temp.exists()){
		    try{
			// delete temporary file
			if(!temp.delete()){  System.out.println("File "  +tempOutputFileName +  " Could not be deleted");} 
		    }
		    catch(Exception e){  e.printStackTrace();}
		}
		status=1; 
	    }
	    // set save status to other value hence nothing gets done
	    else if(promptResult == JOptionPane.CLOSED_OPTION){  status=-5; }
	}
	// if there was no graph to save or the current graph was saved successfully, or the user 
	if(status==1){ 
	    // set ShowTrees.parserLoaded =false;
	    ShowTrees.parserLoaded=false;
	    dispose();
	    disconnectFromServer();
	}
	
	
    }
}








