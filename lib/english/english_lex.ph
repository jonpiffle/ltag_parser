# -*- perl -*-
#--------------------------------------------------
# DEFINES
#--------------------------------------------------

if (!defined &_english_lex_ph) {
    eval 'sub _english_lex_ph {1;}';

    $LEMINPUT  = $ENV{'LEMINPUT'};
    $LEMOUTPUT = $ENV{'LEMOUTPUT'};

    if ((!defined $LEMINPUT) || ($LEMINPUT eq '')) { 
	die "$0: environment variable LEMINPUT has no value\n"; 
    }
    if ((!defined $LEMOUTPUT) || ($LEMOUTPUT eq '')) { 
	die "$0: environment variable LEMOUTPUT has no value\n"; 
    }

    $LANG          = 'english';
    $SYN_FILE      = "$LEMINPUT/syntax/syntax-coded.flat";
    $MORPH_FILE    = "$LEMINPUT/morphology/trunc_morph.flat";
    $COMBINED_FILE = "$LEMOUTPUT/data/$LANG/syntax.flat";
    $POS_LABELS    = "$LEMOUTPUT/lib/$LANG/morph.ph";
    $POS_TBLS      = "$LEMOUTPUT/lib/$LANG/POS.ph";
    $DEFAULTS      = "$LEMOUTPUT/lib/$LANG/defaults.ph";
    $EPSILONS      = "$LEMOUTPUT/lib/$LANG/epsilons.in";
    $TEMPLATES     = "$LEMINPUT/templates.lex";
    $NEW_TEMPLATES = "$LEMOUTPUT/data/$LANG/feat_templates.dat";
}


1;


