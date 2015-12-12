#!@PERLBIN@ -I@LEMHOME@/src/grammar

# see README.data in this directory for explanation of the output file
# formats

use strict;
no strict 'vars';
use integer;

package main;

use English;
use Tree;

require 'tree.ph' 
    or die "Installation botched. Could not find tree.ph\n";

require 'create_project_file.pl' 
    or die "Installation botched. Could not find create_project_file.pl\n";

$verbose   = 0;
$usage     = "usage: $0 <input-grammar> [<model-file>]\n";
$GRAM      = shift or die $usage;
$MODEL     = shift;
$MODELFILE = "";
$MODELFILE = glob($MODEL) if (defined $MODEL);

require 'grammar_defn.pl'
    or die "Installation botched. Could not find grammar_defn.pl\n";

# get the reader program for the grammar
require $READER 
    or die "Installation botched. Could not find $READER\n";

%tree_tbl = ();         # used by yyparse
$tree_list = ();	# used by yyparse
%family_tbl = ();

%label_tbl = ();
@inv_label_tbl;
$label_start = 0;
$label_curindex = $label_start;
$maxwidth_labels = 0;

%subscript_tbl = ();
@inv_subscript_tbl;
$subscript_start = 0;
$subscript_curindex = $subscript_start;
$maxwidth_subscripts = 0;

$num_eps = 0;
$num_top = 0;

sub dump_elementary_trees 
{
    open(ELEM, ">$ELEM_TREES") or die "could not open $ELEM_TREES";
    open(NAME, ">$TREE_NAMES") or die "could not open $TREE_NAMES";
    print STDERR "opening $ELEM_TREES for output\n";
    print STDERR "opening $TREE_NAMES for output\n";
    foreach (keys %tree_tbl) { 
	print NAME "$_\n";
	my($root) = $tree_tbl{$_}->[0];
	my($foot) = sane_foot_value(find_in_tree($root, "footp"));
	my @subst = find_in_tree($root, "substp");
	my @head = find_in_tree($root, "headp");
	my @na = find_in_tree($root, "NA");
	my $subst_s = join(" ", @subst);
	my $subst_pad = ($#subst == -1) ? "" : " ";
	my $head_s = join(" ", @head);
	my $head_pad = ($#head == -1) ? "" : " ";
	my $na_s = join(" ", @na);
	my $na_pad = ($#na == -1) ? "" : " ";
	if (($#subst == -1) && ($#head == -1) && ($#na == -1)) { die "peculiar tree: $_\n"; }
	if (($foot != $Tree::NULL_T) && ($#na == -1)) {
	    print STDERR "warning: foot doesn't have NA constraint in $_\n";
	}
	print ELEM
	    "$root $foot ", $#subst+1, " ", $#head+1, " ", $#na+1, 
	    $subst_pad, $subst_s, 
	    $head_pad, $head_s, 
	    $na_pad, $na_s,
	    "\n";
    }
    close(ELEM);
    close(NAME);
    print STDERR "closing $ELEM_TREES\n";
    print STDERR "closing $TREE_NAMES\n";
}

sub sane_foot_value 
{   my(@foot_list) = @ARG;
    if ($#foot_list == $[-1) { 
	return $Tree::NULL_T; 
    }
    elsif ($#foot_list == 0) { 
	return $foot_list[0]; 
    }
    else { 
	printf STDERR "two footnodes in grammar\n"; 
	return $foot_list[0]; 
    }
}

sub find_in_tree
{   my ($node, $key) = @ARG;
    my (@info);
    my ($value) = $Tree::NULL_T;
    return () if ($node == $Tree::NULL_T);
    my($ptr) = $Tree::tnode_tbl[$node];
    return () if (! $ptr);
    @info = split(/:/, $ptr->{"INFO"});
    shift(@info);
    foreach (@info) {
	$value = $node if (/$key/i);
    }
    my(@left, @right) = ();
    push(@left, find_in_tree($ptr->{"LEFTCHILD"}, $key));
    if ($ptr->{"FLAG"} == $Tree::FALSE) {
	push(@right, find_in_tree($ptr->{"RIGHT"}, $key));
    }
    return ($value == $Tree::NULL_T) ? (@left, @right) : ($value, @left, @right);
}

sub dump_tnode_tbl
{
    open(TNODE, ">$TNODES") or die "could not open $TNODES";
    print STDERR "opening $TNODES for output\n";
    my($max) = $#Tree::tnode_tbl;
    for ($i = $Tree::START_T; $i <= $max; $i++) {
	my($ptr) = $Tree::tnode_tbl[$i];
	next if (! $ptr);
	next if ($ptr == $Tree::NULL_T);
	print STDERR "no info found at node $i\n" if (! $ptr->{"INFO"});
	my(@info) = split(/:/, $ptr->{"INFO"}, 2);
	print STDERR "no info found at node $i\n" if (! $info[0]);
	my($label) = $info[0];
	$label =~ s/(.*)?_.*/$1/go;
	my($subscript) = $info[0];
	if ($subscript =~ /_/) { $subscript =~ s/.*?_(.*)/$1/go; }
	else { $subscript = "_"; }

	my ($label_id, $subscript_id);


	if (!defined $label_tbl{$label}) {
	    $label_tbl{$label} = $label_curindex;
	    $inv_label_tbl[$label_curindex] = $label;
	    $label_curindex++;
	}
	$label_id = $label_tbl{$label};
	my($label_len) = length($label);
	$maxwidth_labels = $label_len if ($maxwidth_labels < $label_len);

	if ($subscript eq "_") {
	    $subscript_id = -1;
	} else {
	    if (!defined $subscript_tbl{$subscript}) {
		$subscript_tbl{$subscript} = $subscript_curindex;
		$inv_subscript_tbl[$subscript_curindex] = $subscript;
		$subscript_curindex++;
	    }
	    $subscript_id = $subscript_tbl{$subscript};
	    my($subscript_len) = length($subscript);
	    $maxwidth_subscripts = $subscript_len if ($maxwidth_subscripts < $subscript_len);
	} 

	print TNODE
	    $ptr->{"LEFTCHILD"}, " ", 
	    $ptr->{"RIGHT"}, " ",
	    $ptr->{"FLAG"}, " ",
	    $label_id, " ", $subscript_id, "\n";
    }
    close(TNODE);
    print STDERR "closing $TNODES\n";
}

sub dump_features
{
    open(FEAT, ">$TREE_FEATS") or die "could not open $TREE_FEATS";
    print STDERR "opening $TREE_FEATS for output\n";
    foreach (keys %tree_tbl) { 
	my($feat) = $tree_tbl{$_}->[1];
	$feat =~ s/\n\n/\n/g;
	print FEAT join("#", split(/\n/, $feat)), "\n";
    }
    close(FEAT);
    print STDERR "closing $TREE_FEATS\n";
}

sub dump_families 
{
   open(FAM, ">$TREE_FAMS") or die "could not open $TREE_FAMS";
   print STDERR "opening $TREE_FAMS for output\n";
   foreach (keys %family_tbl) { 
       print FAM $_, " ";
       @tree_list = @{$family_tbl{$_}};
       print FAM $#tree_list+1, " ";
       my $tree_str = join(" ", @tree_list);
       print FAM "$tree_str\n";
#        foreach (@tree_list) {
# 	   print FAM $_, " ";
#        }
#        print FAM "\n";
   }
   close(FAM);
   print STDERR "closing $TREE_FAMS\n";
}

sub dump_label_tbl
{
    open(LABELS, ">$LABELS") or die "could not open $LABELS";
    print STDERR "opening $LABELS for output\n";
    my($i);
    for ($i = 0; $i < $label_curindex; $i++) {
	print LABELS $inv_label_tbl[$i], "\n";
    }
    close(LABELS);
    print STDERR "closing $LABELS\n";
}

sub dump_subscript_tbl
{
    open(SUBSCRIPTS, ">$SUBSCRIPTS") or die "could not open $SUBSCRIPTS";
    print STDERR "opening $SUBSCRIPTS for output\n";
    my($i);
    for ($i = 0; $i < $subscript_curindex; $i++) {
	print SUBSCRIPTS $inv_subscript_tbl[$i], "\n";
    }
    close(SUBSCRIPTS);
    print STDERR "closing $SUBSCRIPTS\n";
}

# check if the output directory exists
$dir = $EPSILONS;
$dir =~ s|/[^/]*$||;
if (!(-d $dir)) {
    die "Error: cannot proceed: could not find directory $dir\n";
}

# copy the stuff from the lib directory first
copy_epsilons_file();
copy_toplabels_file();
copy_defaults_file();

# suck in all the Xtag tree/family files
foreach $tree (@TREE_FILES) {
    $tree_list = [];
    yylex_file($TREES_DIR . "/" . $tree . $TREEFILE_SUFFIX);
    eval '$status = yyparse;';
    if ($@) { yyerror($@); next; }
}
foreach $family (@FAMILY_FILES) {
    $tree_list = [];
    yylex_file($TREES_DIR . "/" . $family . $TREEFILE_SUFFIX);
    eval '$status = yyparse;';
    if ($@) { yyerror($@); next; }
    $family_tbl{$family} = $tree_list;
}

dump_elementary_trees();
dump_tnode_tbl();
dump_features();
dump_families();
dump_label_tbl();
dump_subscript_tbl();

@tree_keys      = keys %tree_tbl;
@family_keys    = keys %family_tbl;
$num_trees      = $#tree_keys+1;
$num_families   = $#family_keys+1;
$num_treenodes  = $#Tree::tnode_tbl+1;
$num_labels     = $label_curindex;
$num_subscripts = $subscript_curindex;

create_project_file();

open(FT, ">$FULL_TREES") or die "could not open $FULL_TREES";
foreach (keys %tree_tbl) { 
    #print "$_\n";
    Tree::rdump($tree_tbl{$_}->[0], *FT); print FT "\n";
    print STDERR $tree_tbl{$_}->[1], "\n" if $verbose >= 2;
}
close(FT);	 
print STDERR "done\n";

1;

