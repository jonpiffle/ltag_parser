import java.io.*;
import java.net.*;
import java.awt.*;
import java.util.*;
import javax.swing.*;



public class Client
{
    // Output stream to server
    DataOutputStream outputToServer;
    // Input reader for server output
    BufferedReader inputFromServer;
    // Socket for connecting to server
    Socket clientSocket;
    int portNumber;
    String hostName;
    
    public Client(int port, String host){
	portNumber = port;
	hostName = host;
    }

    /*
      Name:
          openConnection
      Args:
          void
      Returns:
          int: 0 => connection cannot be openned
	       1 => connection openned
      Description:
             Opens a new socket to the host given by hostName at the port number specified by portNumber
	     
    */

    public int openConnection(){
	InetAddress host;
	try{    
	    host = InetAddress.getByName(hostName); 
	    //host = InetAddress.getLocalHost();
	}
	catch(UnknownHostException e){
	    e.printStackTrace();
	    return 0;
	}
	try{
	    // create a new connection to server
	    //clientSocket = new Socket(hostName, portNumber);
	    clientSocket = new Socket(host, portNumber);
	    // buffer the input stream from server
	    inputFromServer = new BufferedReader(new InputStreamReader(new DataInputStream(clientSocket.getInputStream())));
	    // setup the output stream to the server
	    outputToServer = new DataOutputStream(clientSocket.getOutputStream());
	    return 1;
	}
	catch(Exception e){
	    e.printStackTrace();
	    return 0;
	}

    }

    /*
      Name:
          closeConnection
      Args:
          void
      Returns:
          int: 0=>could not close connection
	       1=> closed connection successfully
      Description:
               closes the current socket
     */

    public int  closeConnection(){
	// send a signal to terminate connection
	try{ outputToServer.writeBytes(">>done<<\n");  }
	catch(Exception e1){ e1.printStackTrace(); }
	// close the socket
	try{  clientSocket.close();  }
	catch(Exception e2 ){  e2.printStackTrace();   }
	return 1;

    }

    /*
      Name:
          sendPreferences
      Args: 
          String fileName: preferences file
      Returns:
          int: 0=>failed to send preferences
	       1=>succeeded in sending preferences
      Description:
               Opens the filename specified by fileName and sends it to the server.
     */

    public int sendPreferences(String fileName){
	FileInputStream fileStream;
	BufferedReader fileReader;
	String line="";
	// open the file	
	try{
	    fileStream = new FileInputStream(fileName);
	    // read the preferences
	    fileReader= new BufferedReader(new InputStreamReader(fileStream));
	    // fill in the prefs array
	    outputToServer.writeBytes(">>begin prefs<<\n");
	    while(line != null){
		line = fileReader.readLine();
		if(line.startsWith("#") || line.compareTo("")==0){} // ignore the line
		else if(line.compareTo("end") == 0){
		    outputToServer.writeBytes(">>end prefs<<\n");
		    break;
		}
		else{
		    outputToServer.writeBytes(line+"\n");
		}
	    }
	    // check for ready signal
	    line=inputFromServer.readLine();
	    if(line.compareTo("ready")==0){
	    }
	    else{
		System.out.println("server did not send ready message");
	    }
	    return 1;
	}
	catch(Exception e){
	    e.printStackTrace();
	    return 0;
	}
    }

    /*
      Name:
          sendAndReceive
      Args:
          String sentence: sentence to be sent to server for parsing
      Returns:
          String: output from server
      Description:
          Sends a sentence to the parser (server) and then reads the output which is returned
      
     */

    public String[] sendAndReceive(String sentence, JTextArea serverDisplay){
	String line="";
	String stdout="";
	String stderr="";
	String result[] = new String[2];
	InputReaderThread inputReaderThread;
	// send the output to the server
	try{
	    inputReaderThread = new InputReaderThread(inputFromServer, serverDisplay);
	    inputReaderThread.start();
	    outputToServer.writeBytes(sentence);
	    inputReaderThread.join();
	    result = inputReaderThread.result;
	}
	catch(Exception e){
	    e.printStackTrace();
	}
	/*
	  try{
	  // read the output from the server
	  while(line.compareTo(">>end<<") !=0){
	  line=inputFromServer.readLine();
	  // get output from the stderr of the server process
	  if(line.startsWith("STDERR:")){ 
	  serverDisplay.append( line.substring(line.indexOf(":")+1) + "\n");
	  }
	  // check if the line is a termination signal
	  if(line.compareTo(">>TERMINATE<<")==0){
	  stderr=line;
	  }
	  // get the output from  the stdout
	  else{   stdout = stdout + line + "\n"; }
	  }
	  result[0] = stdout;
	  }
	  catch(Exception e){
	  e.printStackTrace();
	  result[0]=null;
	  }
	  result[1]=stderr;
	*/
	return result;
    }
}













