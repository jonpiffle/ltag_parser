import java.io.*;
import java.net.*;
import java.util.*;

public class Connection extends Thread
{
    Socket socket;
    DataOutputStream outputToClient=null;
    BufferedReader inputFromClient=null;
    String inputString;
    String outputString;
    int status=-1;
    boolean terminate=false;
    String prefsFile;

    public Connection(Socket socket, String threadName){
	super(threadName);
	this.socket = socket;
	prefsFile = "/tmp/xtag.prefs"+System.currentTimeMillis();
    }

    
    public Hashtable getPreferences(BufferedReader inputStream){
	String line="";
	Hashtable prefs=new Hashtable(30);
	String param;
	String value;
	// receive preferences from client first
	try{
	    System.out.println("Preferences: " );
	    while(line != null){
		line=inputStream.readLine();
		
		if(line.compareTo(">>end prefs<<")==0){
		    break;
		}
		else{
		    param=line.substring(0, line.indexOf("="));
		    value=line.substring(line.indexOf("=")+1);
		    prefs.put(param, value);
		}
	    }
	}
	catch(IOException e){
	    e.printStackTrace();
	}
	return prefs;
    }
    
    public void run() {
	Parser parser;
	InputManager inputReader;
	String sentences[];
	int numOfSentences;
	int i=0;
	Hashtable preferences;
	try {
	    // setup the input stream from the client to server
	    inputFromClient =  new BufferedReader(new InputStreamReader(new DataInputStream(socket.getInputStream())));
	    // setup the output stream from the server to client
	    outputToClient = new DataOutputStream(socket.getOutputStream());
	    // get the preferences
	    inputString=inputFromClient.readLine();
	    if(inputString.startsWith(">>begin prefs<<")){
		preferences = getPreferences(inputFromClient);
	    }
	    else{
		System.out.println("error: expected prefs");
		return;
	    }
	    // create new parser
	    parser = new Parser(preferences, outputToClient);
	    // configure the parser 
	    if(parser.configParser(prefsFile) !=1){
		System.out.println("error: preferences could not be written. Using defaults");
	    }
	    // send a "ready" signal
	    outputToClient.writeBytes("ready\n");
	    inputString = inputFromClient.readLine();
	    while(inputString !=null){
		// check to see if termination condition is set
		if(terminate){ 
		    outputString=">>TERMINATE<<\n";
		    outputToClient.writeBytes(outputString+">>end<<\n");
		    // clear flag
		    terminate=false;
		}
		if(inputString.compareTo(">>done<<")==0){
		    // close sockets and streams
		    break;
		}
		// check to see if the user is sending new preferences
		else if(inputString.startsWith(">>begin prefs<<")){
		    preferences=getPreferences(inputFromClient);
		    parser.setPrefs(preferences);
		    outputToClient.writeBytes("ready\n");
		}
		else{
		    outputString=parser.parse(inputString+"\n"); 
		    outputToClient.writeBytes(outputString+">>end<<\n");
		    outputToClient.flush();
		}
		inputString = inputFromClient.readLine();
	    }
	    // then terminate the connection
	    inputFromClient.close();
            outputToClient.close();
            socket.close();
	    // delete the preferences file that was being used for this connection
	    try{
		if((new File(prefsFile)).delete()){  System.out.println("temp file: " + prefsFile + " deleted");  }
		else{  System.out.println("temp file: " + prefsFile + " could not be deleted");  }
	    }
	    catch(Exception e){
		e.printStackTrace();
	    }
	    status = 1;
        } 
	catch (Exception e) {
	    status = 0;
            e.printStackTrace();
        }
    }

    
    /*
      Name: sendTerminationWarning
      Args: 
      Returns:
      Description:
                 sets the input string to a termination indicator. This is then sent to the client.
		 The client responds by sending a string indicating that it is done. The Thread is 
		 then terminated when the run method returns.
		 
    */
    
    public void  sendTerminationWarning(){
	terminate=true;

    }
    
    
}





