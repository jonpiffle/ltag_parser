import java.io.*;
import java.net.*;
import java.awt.*;
import java.util.*;
import java.util.Hashtable;
import javax.swing.event.*;
import javax.swing.*;
import java.awt.event.*;


/*
  Name:
  Server
  Description:
      
 */

public class ServerThread extends Thread
{
    int portNnumber;
    ServerSocket serverSocket;
    boolean serverRunning;
    DataOutputStream outputToClient;
    BufferedReader inputFromClient;
    Vector connections;
    String connectionPrefix;
    int numberOfConnections;
    Connection connection;
    int portNumber;
    String name;
    
    public ServerThread(int portNum, String name){
	portNumber = portNum;
	connections = new Vector(20, 10);
	connectionPrefix="Connection:";
	numberOfConnections=0;
	this.name = name;
    }
    
    
    /*
      Name: startServer
      Args:
      Returns:
      Description:
                  Checks to see if the specified port is useable
		  and then creates a socket through which to communicate 
		  with clients
		  
    */

    public void run(){
	startServer();
    }

    public int startServer(){
	int status;
	int clientNumber=0;
	status = 1;
	serverRunning=true;
	// create the socket
	try{
	    serverSocket = new ServerSocket(portNumber);
	}
	catch(IOException e){
	    status =0;
	    return status;
	}
	// if a server socket was created successfully, listen for clients
	while(serverRunning){
	    // create new thread and add it to a vector. necessary for sending termination warnings when the server shuts down
	    clientNumber++;
	    // create new thread to handle new connections and start it
	    try{
		
		System.out.println("Ready to handle requests");
		connection = new Connection(serverSocket.accept(), connectionPrefix+clientNumber);
		connections.addElement(connection);
		connection.start();
	    }
	    catch(Exception e){
		System.out.println("Error Creating Connection");
	    }
	}
	System.out.println("Server Shutting down....");
	return status;
    }
    
    /*
      Name: stopServer
      Args:
      Returns:
      Description:
                 Checks to see if there are clients still connected
		 If there are, it sends a message indicating that server
		 is shutting down. It then closes the sockets and cleans up.
    */

    
    public void stopServer(){
	// stop accepting connections
	serverRunning=false;
	// notify all the connections registered in connections and wait for current jobs to finish
	
	// close the server socket
	try{
	    serverSocket.close();
	}
	catch(Exception e){
	    System.out.println("Error closing socket");
	}
	
    }
    
}

