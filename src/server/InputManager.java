import java.io.*;
import java.net.*;
import java.util.*;

public class InputManager extends Thread
{
    BufferedReader inputReader;
    DataOutputStream outputToClient;
    String output;
    int status=-1;
    boolean debug=false;
    public InputManager(BufferedReader input)
    {
	inputReader = input;
	output="";
	outputToClient=null;
    }
    
    public InputManager(BufferedReader input, boolean debug){
	this(input);
	this.debug = debug;
    }
    
    public InputManager(BufferedReader input, DataOutputStream toClient ){
	this(input);
	outputToClient = toClient;
    }
    
    public void run(){
	
	String line="";
	try{
	    if(outputToClient != null){  // for error streams
		line = inputReader.readLine();
		while(line !=null){
		    outputToClient.writeBytes("STDERR:" + line + "\n");
		    line = inputReader.readLine();
		}
	    }
	    else if(debug){
		line = inputReader.readLine();
		while(line !=null){
		    System.out.println(line);
		    output=output+line+"\n";
		    if(line.startsWith("end")){   break; }
		    line = inputReader.readLine();
		}
	    }
	    else{
		line = inputReader.readLine();
		while(line !=null){
		    output=output+line+"\n";
		    if(line.startsWith("end")){   break; }
		    line = inputReader.readLine();
		}
	    }
	    status=1;
	}
	catch(Exception e){
	    output=">>>ERROR<<<";
	    e.printStackTrace();
	    status=0;
	}
    }
}





