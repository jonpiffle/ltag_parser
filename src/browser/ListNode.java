import java.io.*; 
import java.util.*;
import java.lang.*;

public class ListNode
{
    Object data;
    ListNode next;
    public ListNode(Object d){
	this(d, null);
    }
    public ListNode(Object d, ListNode n){
	data = d;
	next = n;
    }
}
