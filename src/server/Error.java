import javax.swing.*;
import javax.swing.border.*;
import java.awt.event.*;
import javax.swing.event.*;
import java.io.*; 
import java.util.*;
import java.lang.*;
import java.awt.*;

/*
  Class: Error
  Methods:
  Description:
             Creates error messages that are presented in modular windows
 */
public class Error extends JOptionPane
{
    Component parent;
    int errType;
    String errMsg;
    public Error(Component par, int err, String msg){
	parent = par;
	errType=err;
	errMsg = msg;
    }
    
    public int showErrMsg(){
	int status=-1;
	int r;
	switch(errType){
	case 0:{
	    r=showConfirmDialog(parent, errMsg, "", YES_NO_OPTION, INFORMATION_MESSAGE);
	    if(r==NO_OPTION){            status=0; }
	    else if(r==YES_OPTION){      status=1; }
	    else if(r==CLOSED_OPTION){   status=2; }
	    break;
	}
	case 1:{
	    showMessageDialog(this,errMsg, "",ERROR_MESSAGE);
	    break;
	}
	default:{
	    break;
	}
	
	}
	return status;
	
    }
    
}




