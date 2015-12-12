import java.io.*;
import java.net.*;
import java.util.*;
import javax.swing.*;

public class InputReaderThread extends Thread
{
    BufferedReader inputReader;
    DataOutputStream outputToClient;
    String output;
    JTextArea serverDisplay;
    String result[] = new String[2];
    public InputReaderThread(BufferedReader input, JTextArea serverDisplay)
    {
	inputReader = input;
	this.serverDisplay = serverDisplay;
    }

    public void run(){
	
	String line;
	String stdout="";
	String stderr="";
	try{
	    line=inputReader.readLine();
	    while(line.compareTo(">>end<<") !=0){
		// get output from the stderr of the server process
		if(line.startsWith("STDERR:")){ 
		    serverDisplay.append( line.substring(line.indexOf(":")+1) + "\n");
		    serverDisplay.repaint();
		}
		// check if the line is a termination signal
		if(line.compareTo(">>TERMINATE<<")==0){
		    stderr=line;
		}
		// get the output from  the stdout
		else{   stdout = stdout + line + "\n"; }
		line=inputReader.readLine();
	    }
	    result[0] = stdout;
	}
	catch(Exception e){
	    e.printStackTrace();
	    result[0]=null;
	}
	result[1]=stderr;
    }
}





