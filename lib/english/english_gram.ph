# -*- perl -*-
#--------------------------------------------------
# DEFINES
#--------------------------------------------------

if (!defined &_english_gram_ph) {
    eval 'sub _english_gram_ph {1;}';

    $LEMINPUT  = $ENV{'LEMINPUT'};
    $LEMOUTPUT = $ENV{'LEMOUTPUT'};

    if ((!defined $LEMINPUT) || ($LEMINPUT eq '')) { 
	die "$0: environment variable LEMINPUT has no value\n"; 
    }
    if ((!defined $LEMOUTPUT) || ($LEMOUTPUT eq '')) { 
	die "$0: environment variable LEMOUTPUT has no value\n"; 
    }

    $LANG      = 'english';
    $READER    = 'treefile.pl';
    $TREES_DIR = "$LEMINPUT/grammar";
    $TREEFILE_SUFFIX = "\.trees";
    @TREE_FILES = qw(
               lex 
               advs-adjs
               prepositions
               determiners
               conjunctions
               modifiers
               auxs
               neg
               punct
               comparatives
	       	     );
    @FAMILY_FILES = qw(
		 Tnx0V
		 TEnx1V
		 Tnx0Vnx1
		 Tnx0Vpnx1
                 Tnx0VPnx1
		 Tnx0Vnx2nx1
		 Tnx0Vnx1pnx2
                 Tnx0Vnx1Pnx2
		 Tnx0Vnx1s2
		 Tnx0Vpl
		 Tnx0Vplnx1
		 Tnx0Vplnx2nx1
		 Tnx0Vs1
		 Tnx0Vax1
		 Ts0Vnx1
		 Tnx0lVN1
		 Tnx0lVN1Pnx2
		 TItVnx1s2
		 TItVpnx1s2
		 TItVad1s2
		 Tnx0Ax1
		 Tnx0A1s1
		 Ts0Ax1
		 Tnx0BEnx1
		 Tnx0N1
		 Tnx0N1s1
		 Ts0N1
		 Tnx0Pnx1
		 Tnx0Px1
		 Ts0Pnx1
		 Tnx0ARBPnx1
		 Tnx0APnx1
		 Tnx0NPnx1
		 Tnx0PPnx1
		 Tnx0PNaPnx1
		 Ts0ARBPnx1
		 Ts0APnx1
		 Ts0NPnx1
		 Ts0PPnx1
		 Ts0PNaPnx1
		 Ts0V
		 Ts0Vtonx1
		 TXnx0Vs1
		 Tnx0nx1ARB
		 Tnx0VDN1
                 Tnx0VDAN1
                 Tnx0VN1
                 Tnx0VAN1
                 Tnx0VDAN1Pnx2
                 Tnx0VAN1Pnx2
                 Tnx0VN1Pnx2
                 Tnx0VDN1Pnx2
		 TRnx0Vnx1A2
		 TRnx0Vnx1Pnx2
		 TREnx1VA2
		 TREnx1VPnx2
		 Ts0Vs1
		       );
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

