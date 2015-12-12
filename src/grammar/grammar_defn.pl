# -*- perl -*-

my $rcsid = '$Id: grammar_defn.pl,v 1.2 2000/11/05 05:06:38 anoop Exp $';

$numTrees_tag        = "numTrees";
$numFamilies_tag     = "numFamilies";
$numTreenodes_tag    = "numTreenodes";
$numLabels_tag       = "numLabels";
$numSubscripts_tag   = "numSubscripts";
$widthLabels_tag     = "widthLabels";
$widthSubscripts_tag = "widthSubscripts";
$elementaryTrees_tag = "elementaryTrees";
$treeNames_tag       = "treeNames";
$treeNodes_tag       = "treeNodes";
$treeFeatures_tag    = "treeFeatures";
$treeFamilies_tag    = "treeFamilies";
$treeLabels_tag      = "treeLabels";
$treeSubscripts_tag  = "treeSubscripts";
$wordDB_tag          = "wordDatabase";
$numepsilons_tag     = "numEmpty";
$epsilons_tag        = "emptyElements";
$numtoplabels_tag    = "numToplabels";
$toplabels_tag       = "topLabels";
$defaults_tag        = "defaultEntry";
$modelfile_tag       = "modelFile";

# Grammar definition, contains locations of input files 
# and output files 
if (-f $GRAM) {
    require $GRAM;
} else {
    die "Error: Could not find $GRAM\n";
}

sub vars_defined 
{   my(@var) = @_;
    foreach (@var) {
	die "$_ not defined\n" if !defined eval "$_";
    }
}

# check if all the necessary variables are defined in $GRAM
&vars_defined(qw(
		 $READER
		 $TREES_DIR
		 $TREEFILE_SUFFIX
		 @TREE_FILES 
		 @FAMILY_FILES 
		 $EPSILON_IN
		 $TOPLABELS_IN
		 $DEFAULTS_IN
		 $PROJECTFILE   
		 $INPROJECTFILE
		 $ELEM_TREES
		 $TREE_NAMES
		 $TNODES    
		 $TREE_FEATS
		 $TREE_FAMS
		 $LABELS    
		 $SUBSCRIPTS
		 $SYNFILE
		 $EPSILONS     
		 $TOPLABELS
		 $DEFAULTS    
		 $FULL_TREES
		 $PRJ_ELEM_TREES
		 $PRJ_TREE_NAMES
		 $PRJ_TNODES 
		 $PRJ_TREE_FEATS
		 $PRJ_TREE_FAMS
		 $PRJ_LABELS 
		 $PRJ_SUBSCRIPTS
		 $PRJ_SYNFILE   
		 $PRJ_EPSILONS  
		 $PRJ_TOPLABELS
		 $PRJ_DEFAULTS 
		 ));

1;

