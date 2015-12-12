#!@PERLBIN@ -I@LEMHOME@/src/grammar

my $rcsid = '$Id: patch_grammar.pl,v 1.2 2000/11/30 19:11:44 anoop Exp $';

use strict;
no strict 'vars';
use integer;

package main;

$verbose   = 0;
$usage     = "usage: $0 <input-grammar> [<model-file>]\n";
$GRAM      = shift or die $usage;
$MODEL     = shift;
$MODELFILE = "";
$MODELFILE = glob($MODEL) if (defined $MODEL);

$num_trees           = 0;
$num_families        = 0;
$num_treenodes       = 0;
$num_labels          = 0;
$num_subscripts      = 0;
$maxwidth_labels     = 0;
$maxwidth_subscripts = 0;
$num_eps             = 0;
$num_top             = 0;
 
require 'grammar_defn.pl';
require 'create_project_file.pl';

# Grammar definition, contains locations of input files 
# and output files 
if (-f $GRAM) {
    require $GRAM;
} else {
    die "Error: Could not find $GRAM\n";
}

my $grammar = $PROJECTFILE;

## Read the filenames from the grammar file
print STDERR "reading grammar file $grammar\n";
open(F, $grammar) or die "could not open $grammar\n";
while (<F>) {
    chomp;
    if (/^$numTrees_tag/) {
	my ($x, $y) = split('=', $_, 2);
	$num_trees = $y;
    }
    if (/^$numFamilies_tag/) {
	my ($x, $y) = split('=', $_, 2);
	$num_families = $y;
    }
    if (/^$numTreenodes_tag/) {
	my ($x, $y) = split('=', $_, 2);
	$num_treenodes = $y;
    }
    if (/^$numLabels_tag/) {
	my ($x, $y) = split('=', $_, 2);
	$num_labels = $y;
    }
    if (/^$numSubscripts_tag/) {
	my ($x, $y) = split('=', $_, 2);
	$num_subscripts = $y;
    }
    if (/^$widthLabels_tag/) {
	my ($x, $y) = split('=', $_, 2);
	$maxwidth_labels = $y;
    }
    if (/^$widthSubscripts_tag/) {
	my ($x, $y) = split('=', $_, 2);
	$maxwidth_subscripts = $y;
    }
    if (/^$numepsilons_tag/) {
	my ($x, $y) = split('=', $_, 2);
	$num_eps = $y;
    }
    if (/^$numtoplabels_tag/) {
	my ($x, $y) = split('=', $_, 2);
	$num_top = $y;
    }
}
print STDERR "done.\n";
close(F);

# copy the stuff from the lib directory 
copy_epsilons_file();
copy_toplabels_file();
copy_defaults_file();

create_project_file();
