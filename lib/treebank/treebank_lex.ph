# -*- perl -*-
#--------------------------------------------------
# DEFINES
#--------------------------------------------------

if (!defined &_treebank_lex_ph) {
    eval 'sub _treebank_lex_ph {1;}';

    $LEMINPUT  = $ENV{'LEMINPUT'};
    $LEMOUTPUT = $ENV{'LEMOUTPUT'};

    if ((!defined $LEMINPUT) || ($LEMINPUT eq '')) { 
	die "$0: environment variable LEMINPUT has no value\n"; 
    }
    if ((!defined $LEMOUTPUT) || ($LEMOUTPUT eq '')) { 
	die "$0: environment variable LEMOUTPUT has no value\n"; 
    }

    $LANG          = 'treebank';
    $WORD_CUTOFF   = 5;
    $TREE_CUTOFF   = 10;
    $UNKNOWN       = '*unknown*';
    $TOP_LABEL     = '**TOP**';
    $ROOTNODE      = 'root';
    $ANCHOR_LBL    = 'anchor';
    $CORPUS_LOC    = "$LEMINPUT";
    $CORPUS_DIR    = '[0-2][0-9]';
    $CORPUS_FILES  = '*';
    $SYNTAX_FMT    = "<<INDEX>>%s<<ENTRY>>%s<<POS>>%s<<TREES>>%s\n";
    $UNKNOWN_FMT   = "<<INDEX>>%%s<<ENTRY>>%%s<<POS>>%s<<TREES>>%s\n";
    $SYNTAX_FILE   = "$LEMOUTPUT/data/$LANG/syntax.flat";
    $MODEL_FILE    = "$LEMOUTPUT/data/$LANG/model.flat";
    $ETREE_INFILE  = "$LEMINPUT/out/etree.out";
    $SYNDEFAULT    = "$LEMOUTPUT/lib/$LANG/syndefault.in";
    $TOPLABEL_FILE = "$LEMOUTPUT/lib/$LANG/toplabels.in";
    $ETREE_FILE    = "$LEMOUTPUT/lib/$LANG/etree_clean.out";
    $EPSILONS      = "$LEMOUTPUT/lib/$LANG/epsilons.in";
    $GRAMMAR_FILE  = "$LEMOUTPUT/data/$LANG/$LANG.grammar";
    $TREENAMES_TAG = 'treeNames';
    $TREENODES_TAG = 'treeNodes';
    $TREES_TAG     = 'elementaryTrees';
}


1;


