import java.lang.*;
public class Feature
{
    String feature;
    String nodeLabel;
    String nodeBase;
    String type;
    String id;
    boolean matched = false;
    boolean complex=false;
    
    public Feature(String feat, String id){
	feature=feat;
	nodeLabel = feat.substring(0, feat.indexOf(":"));
	if(nodeLabel.indexOf("_") == -1){
	    nodeBase = nodeLabel.substring(0, nodeLabel.indexOf("."));
	}
	else{
	    nodeBase = nodeLabel.substring(0, nodeLabel.indexOf("_"));
	}
	type = feat.substring(feat.indexOf("<") + 1, feat.indexOf(">"));
	this.id = id;
    }
    
}
