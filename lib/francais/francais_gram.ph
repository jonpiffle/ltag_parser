# -*- perl -*-
#--------------------------------------------------
# DEFINES
#--------------------------------------------------

if (!defined &_francais_gram_ph) {
    eval 'sub _francais_gram_ph {1;}';

    $LEMINPUT  = $ENV{'LEMINPUT'};
    $LEMOUTPUT = $ENV{'LEMOUTPUT'};

    if ((!defined $LEMINPUT) || ($LEMINPUT eq '')) { 
	die "$0: environment variable LEMINPUT has no value\n"; 
    }
    if ((!defined $LEMOUTPUT) || ($LEMOUTPUT eq '')) { 
	die "$0: environment variable LEMOUTPUT has no value\n"; 
    }

    $LANG      = 'francais';
    $READER    = 'treefile.pl';
    $TREES_DIR = "$LEMINPUT/grammar";
    $TREEFILE_SUFFIX = "\.trees";
    @TREE_FILES = qw(
		     lex 
		     modif
	       	     );
    @FAMILY_FILES = qw(
		       a0Ad
		       cl0seVden1
		       cl0V-a
		       cl0Va
		       cl0Vcs1
		       cl0Vn1_an2_
		       cl0Vs1
		       n0Ades1
		       n0A
		       n0Apn1
		       n0Aps1
		       n0seVa1
		       n0seVan1
		       n0seVas1
		       n0seVcs1inf
		       n0seVden1
		       n0seVdes1
		       n0seVloc1
		       n0seV
		       n0seVpn1
		       n0seVs1ind
		       n0Va1-a
		       n0Va1-e
		       n0Van1-a
		       n0Van1_den2_
		       n0Van1den2
		       n0Van1-e
		       n0V-a
		       n0Vas1-a
		       n0Vas1-e
		       n0Vcs1ind_an2_
		       n0Vcs1ind
		       n0Vcs1subj_an2_
		       n0Vcs1subj_den2_
		       n0Vcs1subj
		       n0Vcs1subjpn2
		       n0Vden1-a
		       n0Vden1-e
		       n0Vden1pn2-a
		       n0Vdes1-a
		       n0Vdes1-e
		       n0V-e
		       n0Vloc1-a
		       n0Vloc1-e
		       n0V_loc1__sbut2_-a
		       n0V_loc1__sbut2_-e
		       n0Vn1a2
		       n0Vn1_an2_
		       n0Vn1as2
		       n0Vn1_den2_
		       n0Vn1des2
		       n0Vn1_loc2_
		       n0Vn1_loc2__sbut3_
		       n0Vn1
		       n0Vn1pn2
		       n0Vn1sperc2
		       n0Vpn1-a
		       n0Vpn1-e
		       n0Vs1ind_an2_
		       n0Vs1ind_den2_
		       n0Vs1ind
		       n0Vs1subj
		       s0Ad
		       s0Cs1
		       s0N
		       s0Pcs1subj
		       s0Pn1
		       s0Ppp1
		       s0Ps1
		       s0Van1-a
		       s0Van1-e
		       s0V-a
		       s0V-e
		       s0Vn1a2
		       s0Vn1_an2_
		       s0Vn1_den2_
		       s0Vn1
		       v0Adneg
		       v0Ad
		       x0Ad
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

