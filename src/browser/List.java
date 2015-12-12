import java.io.*; 
import java.util.*;
import java.lang.*; 

public class List
{
    ListNode head;
    ListNode current;
    ListNode tail;
    int size=0;
    // create the empty list
    public List()
    {
	    head = new ListNode(null);
	    current=tail=head;
    }
    
    public List(Object obj)
    {
	    head = new ListNode(null);
	    head.next= new ListNode(obj);
	    current = head.next;
    }
    
    public boolean isEmpty()
    {
	    return head.next == null;
    }
    public void makeEmpty()
    {
	    head.next = null;
    }
    
    
    public void put(Object obj)
    {
	ListNode tail;
	tail = head;
	// go to the last element of the list
	while(tail.next != null)
	    tail = tail.next;
	tail.next=(new ListNode(obj));
    }
    // concatenates list with "this" list
    public void concatenate(List list){
	ListNode l;
	// check to see that the list is NOT null
	if(list != null){
	    // if the list that we are concatenating is not empty
	    l = list.head;
	    int i=0;
	    while(l.next !=null){
		put(l.next.data);
		l=l.next;
	    }
	    list=null;
	}
    }
    
    public ListNode getNext()
    {
	    current = current.next;
	    if(current !=null)
	        return current;
	    else
	        return null;
    }
    
    public void printList()
    {
	    ListNode start = head;
	    int i=0;
	    while(start.next != null){
	        i++;
	        System.out.println((String)start.next.data);
	        start = start.next;
	    }
	    System.err.println("Total: " + i);
    }

    public void resetNext()
    {
	    current=head;
    }
    
    
    public void  replaceString(String pattern, String str)
    {
	    int i=0;
	    int count = 0;
	    int index=-1;
	    int index2 = -1;
	    String token="";
	    String data = "";
	    String newdata="";
	    ListNode start = head;
	    String pre="";
	    String post="";

	    while(start.next !=null)
	    {
	        // get the data
	        data = (String)start.next.data;
	        // tokenize the sting with pattern as the separator
	        index = data.indexOf(pattern);
	    
	        if(index !=-1)
	        {
		        pre = data.substring(0, index);
		        post = data.substring(index+pattern.length());
		        index2 = pre.lastIndexOf(" ");
		        if(index2 != -1)
		        {
		            pre = data.substring(0, index2);
		        }
		        newdata=pre + str + post;
		        start.next.data=newdata;
	        }
	        index=index2=-1;
	        start=start.next;
	    }
    }
    
    
    public void advance()
    {
	    current = current.next;
    }
} 



