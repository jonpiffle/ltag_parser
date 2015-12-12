# -*- perl -*-
#--------------------------------------------------
# DEFINES
#--------------------------------------------------

package main;

if (!defined &_fzap_ph) {
    eval 'sub _fzap_ph {1;}';

    $LEMINPUT  = $ENV{'LEMINPUT'};
    $LEMOUTPUT = $ENV{'LEMOUTPUT'};

    if ((!defined $LEMINPUT) || ($LEMINPUT eq '')) { 
        die "$0: environment variable LEMINPUT has no value\n"; 
    }
    if ((!defined $LEMOUTPUT) || ($LEMOUTPUT eq '')) { 
        die "$0: environment variable LEMOUTPUT has no value\n"; 
    }

    $LANG      = 'english_tiny';
    $TREES_DIR = "$LEMINPUT/grammar";
    $TREEFILE_SUFFIX = "\.trees";
    @TREE_FILES = qw(
		     lex
		     advs-adjs
		     punct
		     comparatives
		     );
    @FAMILY_FILES = qw(
		       Tnx0V
		       Tnx0Vnx1
		       Tnx0Vnx1nx2
		       );
    $TEMPLATE_FILE = "$LEMINPUT/templates.lex";
    $EPSILON_IN    = "$LEMOUTPUT/lib/english/epsilons.in";
    $TOPLABELS_IN  = "$LEMOUTPUT/lib/english/toplabels.in";
    $DEFAULTS_IN   = "$LEMOUTPUT/lib/english/syndefault.in";
    $PROJECTFILE   = "$LEMOUTPUT/data/$LANG/$LANG.grammar";
    $INPROJECTFILE = "$LEMOUTPUT/data/$LANG/$LANG.grammar.in";
    $ELEM_TREES    = "$LEMOUTPUT/data/$LANG/trees.dat";
    $TREE_NAMES    = "$LEMOUTPUT/data/$LANG/treenames.dat";
    $TNODES        = "$LEMOUTPUT/data/$LANG/treenodes.dat";
    $TREE_FEATS    = "$LEMOUTPUT/data/$LANG/treefeats.dat";
    $TREE_FAMS     = "$LEMOUTPUT/data/$LANG/treefams.dat";
    $LABELS        = "$LEMOUTPUT/data/$LANG/labels.dat";
    $SUBSCRIPTS    = "$LEMOUTPUT/data/$LANG/subscripts.dat";
    $SYNFILE       = "$LEMOUTPUT/data/$LANG/syntax.db";
    $EPSILONS      = "$LEMOUTPUT/data/$LANG/epsilons.dat";
    $TOPLABELS     = "$LEMOUTPUT/data/$LANG/toplabels.dat";
    $DEFAULTS      = "$LEMOUTPUT/data/$LANG/syndefault.dat";
    $FULL_TREES    = "$LEMOUTPUT/data/$LANG/xtag.trees.dat";
}

1;

