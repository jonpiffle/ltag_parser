# -*- perl -*-
#--------------------------------------------------
# DEFINES
#--------------------------------------------------

if (!defined &_english_gram_ph) {
    eval 'sub _english_gram_ph {1;}';

    $LEMOUTPUT = $ENV{'LEMOUTPUT'};

    if ((!defined $LEMOUTPUT) || ($LEMOUTPUT eq '')) { 
	die "$0: environment variable LEMOUTPUT has no value\n"; 
    }

    $LANG      = 'treebank';
    $READER    = 'etrees.pl';
    $TREES_DIR = "$LEMOUTPUT/lib/$LANG/";
    $TREEFILE_SUFFIX = "\.out";
    @TREE_FILES = qw(
		     etree_clean
	       	     );
    @FAMILY_FILES = ();
    $EPSILON_IN     = "$LEMOUTPUT/lib/$LANG/epsilons.in";
    $TOPLABELS_IN   = "$LEMOUTPUT/lib/$LANG/toplabels.in";
    $DEFAULTS_IN    = "$LEMOUTPUT/lib/$LANG/syndefault.in";
    $PROJECTFILE    = "$LEMOUTPUT/data/$LANG/$LANG.grammar";
    $INPROJECTFILE  = "$LEMOUTPUT/data/$LANG/$LANG.grammar.in";
    $ELEM_TREES     = "$LEMOUTPUT/data/$LANG/trees.dat";
    $TREE_NAMES     = "$LEMOUTPUT/data/$LANG/treenames.dat";
    $TNODES         = "$LEMOUTPUT/data/$LANG/treenodes.dat";
    $TREE_FEATS     = "$LEMOUTPUT/data/$LANG/treefeats.dat";
    $TREE_FAMS      = "$LEMOUTPUT/data/$LANG/treefams.dat";
    $LABELS         = "$LEMOUTPUT/data/$LANG/labels.dat";
    $SUBSCRIPTS     = "$LEMOUTPUT/data/$LANG/subscripts.dat";
    $SYNFILE        = "$LEMOUTPUT/data/$LANG/syntax.db";
    $EPSILONS       = "$LEMOUTPUT/data/$LANG/epsilons.dat";
    $TOPLABELS      = "$LEMOUTPUT/data/$LANG/toplabels.dat";
    $DEFAULTS       = "$LEMOUTPUT/data/$LANG/syndefault.dat";
    $PRJ_ELEM_TREES = "\@LEMHOME\@/data/$LANG/trees.dat"; 
    $PRJ_TREE_NAMES = "\@LEMHOME\@/data/$LANG/treenames.dat";
    $PRJ_TNODES     = "\@LEMHOME\@/data/$LANG/treenodes.dat";
    $PRJ_TREE_FEATS = "\@LEMHOME\@/data/$LANG/treefeats.dat";
    $PRJ_TREE_FAMS  = "\@LEMHOME\@/data/$LANG/treefams.dat";
    $PRJ_LABELS     = "\@LEMHOME\@/data/$LANG/labels.dat";
    $PRJ_SUBSCRIPTS = "\@LEMHOME\@/data/$LANG/subscripts.dat";
    $PRJ_SYNFILE    = "\@LEMHOME\@/data/$LANG/syntax.db";
    $PRJ_EPSILONS   = "\@LEMHOME\@/data/$LANG/epsilons.dat";
    $PRJ_TOPLABELS  = "\@LEMHOME\@/data/$LANG/toplabels.dat";
    $PRJ_DEFAULTS   = "\@LEMHOME\@/data/$LANG/syndefault.dat";

    $FULL_TREES    = "$LEMOUTPUT/data/$LANG/xtag.trees.dat";
}

1;

