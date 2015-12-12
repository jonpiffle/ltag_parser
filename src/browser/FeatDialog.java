import javax.swing.*;
import javax.swing.border.*;
import java.awt.event.*;
import javax.swing.event.*;
import java.io.*; 
import java.util.*;
import java.lang.*;
import java.awt.*;

public class FeatDialog extends JDialog
{

    JButton okButton, cancelButton;
    JPanel panel;
    JTextField textField;
    JLabel prompt;
    int value;
    Container contentPane;
    String label;
    FontMetrics fm;
    int charWidth ;
    int charHeight;
    JFrame parent;
    int width;
    int height;
    public FeatDialog(JFrame p, String title, Font currentFont){
	super(p, title, true);
	parent = p;
	// set the font
	setFont(currentFont);
	// set the fontmetrics
	fm = getFontMetrics(currentFont);
	// create textwindow
	textField = new JTextField();
	// create buttons
	okButton = new JButton("OK");
	cancelButton = new JButton("Cancel");
	// register event handlers
	okButton.addActionListener(new FeatDialogEventHandler());
	cancelButton.addActionListener(new FeatDialogEventHandler());
	// get contents pane
	contentPane = getContentPane();
	contentPane.setLayout(null);
	label = "Maximum number of features to be displayed";
	prompt = new JLabel(label);
        charWidth = fm.stringWidth("X");
	charHeight = fm.getHeight();
	// set the bounds
	prompt.setBounds(20,20,(charWidth+3)*label.length(), charHeight);
	textField.setBounds(20+(charWidth+3)*label.length(), 20, (charWidth+5)*4, charHeight);
	width = 20+(charWidth+3)*label.length()  + (charWidth+5)*4 ;
	cancelButton.setBounds(width - ((charWidth+5)*6), 20+(charHeight+5),((charWidth+7)*6),charHeight );
	okButton.setBounds(width-((charWidth+7)*6)-((charWidth+5)*5), 
			   20+(charHeight+5), (charWidth+3)*6, charHeight);
	
	// add textwindow and buttons
	contentPane.add(prompt);
	contentPane.add(textField);
	contentPane.add(okButton);
	contentPane.add(cancelButton);
        width= cancelButton.getX()+cancelButton.getWidth();
	height = cancelButton.getY() + cancelButton.getHeight();
	setSize(width+20, + height+30);
    }
    
    public void showWindow(){
	setLocationRelativeTo(parent);
	setVisible(true);
    }
    
    class FeatDialogEventHandler implements ActionListener{
	public void actionPerformed(ActionEvent event){
	    Object obj = event.getSource();
	    if(obj == okButton){
		handleOkButton();
	    }
	    else if(obj == cancelButton){
		handleCancelButton();
	    }
	}
	
    }
    
    public void handleCancelButton(){
	// set value to -1 to indicate a cancel operation
	value =-1;
	// set to invisible
	setVisible(false);
    }
    
    public void handleOkButton(){
	String enteredValue;
	//int errStatus;
	// read the contents of the textfield
	enteredValue=textField.getText();
	// convert contents into an integer if possible
	if(enteredValue != null){
	    try{
		value = Integer.parseInt(enteredValue);
		if(value < 1 || value > 30){
		    Error err = new Error(this, 1, "Please enter a number between 1 and 30");
		    err.showErrMsg();
		}
		else{
		    setVisible(false);
		}
	    }
	    catch(Exception e){
		Error err = new Error(this, 1, "Please enter a number between 1 and 30");
		err.showErrMsg();
		value=-1;
	    }
	}
	else{
	    System.out.println("Error: Expects integer");
	    value = -1;
	}
	
    }
    
    public static void main(String args[]){
	// create a new JFrame
	JFrame test = new JFrame("Test Frame");
       	// create new dialog
	
	final FeatDialog dialog = new FeatDialog(test, "Feat NUM", test.getContentPane().getFont());
	// associate dialog box
	JButton b = new JButton("Show Dialog");
	b.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent e) {
		    dialog.showWindow();
		}
	    });
	test.setSize(400,400);
	test.getContentPane().add(b);
	test.setVisible(true);
    }
    

}

