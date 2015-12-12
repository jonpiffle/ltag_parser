import java.io.*;
import java.net.*;
import java.util.*;

public class ClientWindowThread extends Thread
{
    ClientWindow c;
    ShowTrees st;
    public ClientWindowThread(ShowTrees st){
	this.st = st;
	c = new ClientWindow(st);
	
    }

    public int initClientWindow(){
	return c.initClientWindow();
    }
    
    public void run(){
	c.setVisible(true);
    }
    public void stopClient(){
	Error error;
	int promptResult;
	if(c.busy){ 
	    error = new Error(st, 0, "Client is busy. Do you Want to Wait?");
	    promptResult = error.showErrMsg();  
	    if (promptResult == 0) { // terminate client and discard any output
	    }
	    else if(promptResult==1){} // do nothing 
	    else if(promptResult == 2){} // do nothing
	}
	else{	c.closeParserActionHandler(); }
    }

}
