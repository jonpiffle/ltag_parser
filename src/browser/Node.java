import java.awt.Label;
//import javax.swing.JLabel;
import java.awt.PopupMenu;
import java.awt.Color;
import java.awt.event.MouseEvent;
import java.awt.event.MouseAdapter;
import java.awt.Font;

public class Node extends Label
{
    ListNode listnode;
    PopupMenu popup;
    MyMenuItem m, currentDtrPair;
    String nodeLabel;
    String label="";
    TreeNode treeNode;
    boolean currentDtrPairSet=false;
    String tag, tag2, subscript;
    int count;
    public Node(String nodelabel, List menuItems, int X, int Y, int width, int height, TreeNode tn){
	super();	
	//setFont(f);
	listnode = menuItems.head;
	treeNode = tn; // pointer to TreeNode object represented by this node which is drawn
	nodeLabel = nodelabel.replace('', 'e');
	/*
	  if(nodeLabel.indexOf("_") != -1){
	  tag=nodeLabel.substring(0, nodeLabel.indexOf("_"));
	  subscript = nodeLabel.substring(nodeLabel.indexOf("_")+1,nodeLabel.indexOf("_")+2 );
	  
	  if(nodeLabel.indexOf("+") != -1){
	  //color = "blue";
	  tag2 = nodeLabel.substring(nodeLabel.indexOf("_")+2,nodeLabel.indexOf("+"));
	  }
	  else if(nodeLabel.indexOf("*") != -1 ){
	  //color = "blue";
	  tag2 = nodeLabel.substring(nodeLabel.indexOf("_")+2,nodeLabel.indexOf("*"));
	  }
	  else{
	  //color= black;
	  tag2 = nodeLabel.substring(nodeLabel.indexOf("_")+2);
	  }
	  nodeLabel = "<html>" + tag + "<sub>   " + subscript +"</sub></html>";// + tag2;
	  }
	*/
	count=0;
	setText(nodeLabel);
	popup = new PopupMenu(); 
	setAlignment(CENTER);
	//setHorizontalAlignment(CENTER); 
	setBounds(X,Y,width,height);
	while(listnode.next != null){
	    m = (MyMenuItem)listnode.next.data;   
	    popup.add(m);
	    // get the label and check to see if it is "Collapse Node", "Expand Node" or some other alternative daugther pair
	    label = m.getLabel();
	    if(label.startsWith("start")){
		count++;
	    }
	    if(!label.startsWith("Collapse") && !label.startsWith("Expand") && !label.startsWith("start")){
		//if(!currentDtrPairSet){ currentDtrPair=m; m.setEnabled(false); currentDtrPairSet=true; }
		count++;
	    }
	    listnode = listnode.next;
	    
	}
	if(count > 1){
	    setForeground(Color.red);
	}
	addMouseListener(new MousePressedHandler(this));
	add(popup);
    }
    
    class MousePressedHandler extends MouseAdapter{
        Node node;
        public MousePressedHandler(Node n){
            node= n;
        }
	public void mousePressed(MouseEvent e){
	    popup.show(node, e.getX(), e.getY());
	}
	
    }
}

