import java.io.*;
import java.net.*;
import java.awt.*;
import java.util.*;
import javax.swing.event.*;
import javax.swing.*;
import java.awt.event.*;


/*
  Name:
      ServerWindow
  Description:
      Class defining user interface for configuring and starting server
*/



public class ServerWindow extends JFrame
{

    JMenuBar menubar;
    JMenu parser;
    JMenuItem startParser, stopParser, configParser, quit;
    ServerConfigWindow dialog;
    String defaultsFileName;
    Server currentServer;
    String serverCmd[];
    public ServerWindow(){
	super("Parser Server");
	serverCmd = new String[4];
	//serverCmd[0] = "/usr/local/jdk1.2.2/bin/java";
	serverCmd[0] ="/pkg/j/jdk1.3/bin/java";
	serverCmd[1] = "-classpath";
	//serverCmd[2] = "/home/kimanzi/ParserServer";
	serverCmd[2] = "/mnt/linc/xtag/work/moses/ParserServer";
	serverCmd[3]="Server";
	// create menubar
	menubar = new JMenuBar();
	// create parser menu
	parser = new JMenu("Parser");
	// create menuitems
	startParser = new JMenuItem("Start Parser");
	stopParser = new JMenuItem("Stop Parser");
	configParser = new JMenuItem("Configure Parser");
	quit = new JMenuItem("Quit");
	// create an event listener
	ServerWindowEventHandler eventListener = new ServerWindowEventHandler();
       	// register event handlers for the menuitems
	startParser.addActionListener(eventListener);
	stopParser.addActionListener(eventListener);
	configParser.addActionListener(eventListener);
	quit.addActionListener(eventListener);
	// add the menu items
	parser.add(startParser);
	parser.add(stopParser);
	parser.add(configParser);
	parser.add(quit);
	// add the menu to the menubar
	menubar.add(parser);
	setJMenuBar(menubar);
	setSize(400,400);
	dialog = null;
	defaultsFileName="";
    }


    class ServerWindowEventHandler implements ActionListener{
	public void actionPerformed(ActionEvent event){
	    Object obj = event.getSource();
	    if(obj == startParser){
		start();
	    }
	    else if(obj == stopParser){
		stop();
	    }
	    else if(obj == configParser){
		loadConfigWindow();
	    }
	    else if(obj == quit){
		System.exit(0);
	    }
	}
    }

    /*
      Name: 
          start
      Args:
      Returns:
      Description:
                 Creates a new server object
		 Reads the defaults for the parser
		 initialized the server
		 starts parser
      
     */
    

    public void start(){
	// get the runtime
	Runtime r =Runtime.getRuntime();
	Process serverP;
	// fork out a new process that will run the server
	try{
	    serverP = r.exec(serverCmd);
	    (new InputManager(new BufferedReader(new InputStreamReader(serverP.getInputStream())), true)).start();
	}
	catch(IOException e){
	    e.printStackTrace();
	}
    }
    
    /*
      Name: 
          stop
      Args:
      Returns:
      Description:
          checks to see if there is a server at the specified server port
	  if there is, it connects to the server and sends a termination signal
    */
    
    public void stop(){
	
    }

    /*
      Name: loadConfigWindow
      Args: 
      Returns:
      Description:
              Create a new configuration window
	      Wait for user to close config window
      
     */
    
    public void loadConfigWindow(){
	// create a new dialog
	dialog = new ServerConfigWindow(this, "Parser Options");
	dialog.setVisible(true);
	dialog.dispose();
    }

    
}


