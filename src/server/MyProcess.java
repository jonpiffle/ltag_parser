import java.io.*;
import java.net.*;
import java.util.*;


public class MyProcess{

    Process process;
    BufferedReader stdOut;
    BufferedReader stdErr;
    OutputStream stdIn;

    public MyProcess(Process p, BufferedReader out, BufferedReader err, OutputStream in){
	process=p;
	stdOut=out;
	stdErr=err;
	stdIn = in;
    }

}
