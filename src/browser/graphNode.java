public class graphNode
{
    String nodeID, daughters, treeName, nodeLabel, position, rootLabel, footLabel, nodeType;
    List features;
    boolean isInserted;
    public graphNode(String nI,String dtrs, String tN,String nL ,String pos, 
		 String rL, String fL, String nT, List featList, boolean insert){
        nodeID= nI;
	daughters = dtrs;
        treeName = tN;
        nodeLabel=nL;
        position = pos;
        rootLabel = rL;
        footLabel = fL;
        nodeType = nT;
	features = featList;
	isInserted = insert;
    }
}
