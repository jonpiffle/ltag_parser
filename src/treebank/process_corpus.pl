#!/usr/bin/perl -w -I@LEMHOME@/src/treebank

# Copyright (c) 2000 Anoop Sarkar
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License 
# as published by the Free Software Foundation; either version 2 of 
# the License, or (at your option) any later version. 
# 
# The file COPYING in the current directory has a copy of the GPL. 
#
# This code comes WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#

my $rcsid   = '$Id: process_corpus.pl,v 1.12 2001/04/10 18:41:34 anoop Exp $'; #'

# takes as input a project file containing various filenames and produces:
# 1. syntax file for the parser 
# 2. a file containing the TOP labels for the parser
# 3. a file containing syntax entries for the *unknown* word token
# 4. a file containing attachment counts
#
# Sample Input:
#
# 27 Leisure NN  m_NP*_anchor@=8_0      anchor 1 root  5 of      IN  m_NP*_anchor@_NPs=3_0  anchor
# 28 &       CC  s_CC@=5_1              CC     1 1_   29 Arts    NNS c_NP*_CCs@_anchor@=6_1 anchor
# 29 Arts    NNS c_NP*_CCs@_anchor@=6_1 anchor 1 1_0_ 27 Leisure NN  m_NP*_anchor@=8_0      anchor
#
# Output:
#
#    <<INDEX>>Leisure<<ENTRY>>Leisure<<POS>>anchor<<TREES>>m_NP*_anchor@=8_0 
#    <<INDEX>>&<<ENTRY>>&<<POS>>CC<<TREES>>s_CC@=5_1 
#

my $verbose = 0;
my $usage   = "usage: $0 <project-file>\n";
my $project = shift or die $usage;

print STDERR "$rcsid\n" if ($verbose > 1);

require $project
    or die "could not load $project\n";

sub vars_defined 
{   my(@var) = @_;
    foreach (@var) {
	die "$_ not defined\n" if !defined eval "$_";
    }
}

# check if all the necessary variables are defined in $project
&vars_defined(qw(
		 $WORD_CUTOFF
		 $TREE_CUTOFF
		 $UNKNOWN
		 $TOP_LABEL
		 $ROOTNODE
		 $CORPUS_LOC
		 $CORPUS_DIR
		 $SYNDEFAULT
		 $WORD_CUTOFF
		 $TREE_CUTOFF
		 $SYNTAX_FMT
		 $UNKNOWN_FMT
		 $UNKNOWN
		 $TOP_LABEL
		 $TOPLABEL_FILE
		 $ETREE_FILE
		 $SYNTAX_FILE
		 $MODEL_FILE
		 $GRAMMAR_FILE
		 $TREENAMES_TAG
		 $TREENODES_TAG
		 $TREES_TAG
		 ));

print STDERR "WORD_CUTOFF=$WORD_CUTOFF\n"     if ($verbose > 1);
print STDERR "TREE_CUTOFF=$TREE_CUTOFF\n"     if ($verbose > 1);
print STDERR "UNKNOWN=$UNKNOWN\n"             if ($verbose > 1);
print STDERR "TOP_LABEL=$TOP_LABEL\n"         if ($verbose > 1);
print STDERR "CORPUS_LOC=$CORPUS_LOC\n"       if ($verbose > 1);
print STDERR "CORPUS_DIR=$CORPUS_DIR\n"       if ($verbose > 1);
print STDERR "SYNDEFAULT=$SYNDEFAULT\n"       if ($verbose > 1);
print STDERR "WORD_CUTOFF=$WORD_CUTOFF\n"     if ($verbose > 1);
print STDERR "TREE_CUTOFF=$TREE_CUTOFF\n"     if ($verbose > 1);
print STDERR "SYNTAX_FMT=$SYNTAX_FMT\n"       if ($verbose > 1);
print STDERR "UNKNOWN_FMT=$UNKNOWN_FMT\n"     if ($verbose > 1);
print STDERR "UNKNOWN=$UNKNOWN\n"             if ($verbose > 1);
print STDERR "TOP_LABEL=$TOP_LABEL\n"         if ($verbose > 1);
print STDERR "TOPLABEL_FILE=$TOPLABEL_FILE\n" if ($verbose > 1);
print STDERR "ETREE_FILE=$ETREE_FILE\n"       if ($verbose > 1);
print STDERR "SYNTAX_FILE=$SYNTAX_FILE\n"     if ($verbose > 1);
print STDERR "MODEL_FILE=$MODEL_FILE\n"       if ($verbose > 1);
print STDERR "ROOTNODE=$ROOTNODE\n"           if ($verbose > 1);

my $corpus_loc    = $CORPUS_LOC;
my $filedes       = $CORPUS_DIR;
my $syndefault    = $SYNDEFAULT;
my $word_cutoff   = $WORD_CUTOFF;
my $tree_cutoff   = $TREE_CUTOFF;
my $fmt_string    = $SYNTAX_FMT;
my $unknown_fmt   = $UNKNOWN_FMT;
my $unknown       = $UNKNOWN;
my $TOP           = $TOP_LABEL;
my $toplabel_file = $TOPLABEL_FILE;
my $etree_file    = $ETREE_FILE;
my $syntax_file   = $SYNTAX_FILE;

my @toplabel        = ();
my %toptbl          = ();
my %toplabels_found = ();

my %wc  = ();
my %tc  = ();
my %syn = ();
my %ins = ();

## Get source to convert Gorn notation in the corpus to treenodes in
## the grammar
require 'replace_treelabels.pl'
   or die "could not find replace_treelabels.pl\n";
initialize_replace_treelabels($project);

## Get code for processing all counts for creating the probability
## model
require 'process_counts.pl'
   or die "could not find process_counts.pl\n";

## Get top labels from $etree_file
print STDERR "reading top labels from $etree_file\n";
open(F, $etree_file) or die "could not open $etree_file\n";
while (<F>)
{
    chomp;
    $ln++;
    if (/^tree_number/) {
	m/^tree_number=(\d+)/o;
	$treenum = $1;
	print STDERR "found tree number: $treenum\n" if ($verbose > 1);
	$nextline = <F>;
	chomp($nextline);
	if ((!defined $nextline) or ($nextline eq '')) {
	    die "file $etree_file is too short at line: $ln\n";
	}
	my $top = $nextline;
	$top =~ s/^\(([^ \)]*).*$/$1/;
	$top =~ s/([^:]*).*$/$1/;
	if (!defined $toptbl{$top}) {
	    $toptbl{$top} = 1;
	    print STDERR "new top_label from $etree_file: $top\n" if ($verbose);
	}
	$toplabel[$treenum] = $top;
    }
}
close(F);
print STDERR "done.\n";

my $fulldes = "$corpus_loc/deriv.long/$filedes";
my @dirs    = glob($fulldes);
my @files   = ();

print STDERR "reading corpus from $fulldes\n";

foreach $dir (@dirs) {
    push(@files, glob("$dir/*"));
}

my $tick = 0;
my $modval = 10000;
foreach $filename (@files)  {
    open(F, $filename) or die "could not find $filename\n";
    my $ln  = 0;
    while (<F>) 
    {
	chomp;
	$ln++;
	$tick++;
	if (($verbose == 0) && (($tick % $modval) == 0)) {
	    print STDERR ".";
	}
	next if /^\s*$/;
	my @line = split;
	next if ($line[3] eq '-');

# Sample Input:
#
# 27 Leisure NN  m_NP*_anchor@=8_0      anchor 1 root  5 of      IN  m_NP*_anchor@_NPs=3_0  anchor
# 28 &       CC  s_CC@=5_1              CC     1 1_   29 Arts    NNS c_NP*_CCs@_anchor@=6_1 anchor
# 29 Arts    NNS c_NP*_CCs@_anchor@=6_1 anchor 1 1_0_ 27 Leisure NN  m_NP*_anchor@=8_0      anchor
#
# 0  1       2   3                      4      5 6    7  8       9   10                     11

## word, tree adjoins/substitutes into depword, deptree at treenode (in deptree)

	my $word         = $line[1];
	my $POS          = $line[2];
	my $treename     = $line[3];
	my $anchor       = $line[4];
	my $gornaddr     = $line[6];
	my $depword      = $line[8];
	my $depPOS       = $line[9];
	my $deptreename  = $line[10];

	# pick only the tree_number as the name of the tree (see etrees.y)
	my $tree = $treename;
	$tree =~ s/^.*=(\d+)_.*$/$1/;

	my $deptree = $deptreename;
	$deptree =~ s/^.*=(\d+)_.*$/$1/;

	if (defined $wc{$word}) { $wc{$word}++; }
	else { $wc{$word} = 1; }

	if (defined $tc{$tree}) { $tc{$tree}++; }
	else { $tc{$tree} = 1; }

	my $value = "$anchor\201$tree";
	if (defined $syn{$word}) { 
	    if (!defined $ins{"$word\201$value"}) {
		push(@{$syn{$word}}, $value);
		$ins{"$word\201$value"} = 1;
	    }
	} else { 
	    $syn{$word} = [ $value ]; 
	    $ins{"$word\201$value"} = 1;
	    print STDERR "new word: $word\n" if ($verbose);
	}

	## Store all distinct TOP labels found
	if ($depword eq $TOP) {
	    print STDERR "$tree: $toplabel[$tree]\n" if ($verbose > 1);
	    my $label = $toplabel[$tree];
	    if (!defined $toplabels_found{$label}) {
		print STDERR "found top label: $label\n" if ($verbose);
		$toplabels_found{$label} = 1;
	    }
	}

	## Map the Gorn address of node to actual treenode
	my $treenode = '';
	if ($depword ne $TOP) {
	    if (($gornaddr eq $ROOTNODE) || ($gornaddr =~ /^\d+_/)) {
		$treenode = find_treenode(treename_to_index($deptree), $gornaddr);
	    } else {
		$treenode = int($gornaddr);
		die "bad Gorn address at line $ln\n" if ($verbose && ($gornaddr !=~ /^\d+$/));
	    }
	}

	## Store counts
	## word, POS, tree, treenode, depword, depPOS, deptree
	store_counts($word, $POS, $tree, $anchor, $treenode, $depword, $depPOS, $deptree);

    }
    close(F);
}
print STDERR "\n" if ($verbose == 0);
print STDERR "done.\n";

## Print out to $toplabel_file all the top labels found
print STDERR "writing top labels to $toplabel_file\n";
open(F, ">$toplabel_file") or die "could not open $toplabel_file\n";
my $top;
foreach $top (keys %toplabels_found)
{
    print F "$top #NONE\n";
}
close(F);
print STDERR "done.\n";

%ins = ();
my $index;
foreach $index (keys %syn) {

    next if ($index eq $unknown);

    if ($wc{$index} <= $word_cutoff) {
	print STDERR "removing word: $index\n" if ($verbose);
	my @valuelist = @{$syn{$index}};
	my $value;
	foreach $value (@valuelist) {
	    if (defined $syn{$unknown}) { 
		if (!defined $ins{$value}) {
		    push(@{$syn{$unknown}}, $value);
		    print STDERR "new $unknown: $value\n" if ($verbose);
		    $ins{$value} = 1;
		}
	    } else { 
		$syn{$unknown} = [ $value ]; 
		print STDERR "new $unknown: $value\n" if ($verbose);
		$ins{$value} = 1;
	    }
	}
	$syn{$index} = [];
    }
}

sub is_unknown_wordtype
{
    my($index) = @_;
    return($#{$syn{$index}} < 0);
}

print STDERR "writing syntax entries to $syntax_file\n";
open(O, ">$syntax_file") or die "could not open $syntax_file\n";
print STDERR "writing entries for $unknown to $syndefault\n";
open(F, ">$syndefault") or die "could not open $syndefault\n";
foreach $index (keys %syn) {

    next if (is_unknown_wordtype($index));
    my @valuelist = @{$syn{$index}};
    #next if ($#valuelist < 0);

    my %anchor_tbl = ();
    my $value;

    foreach $value (@valuelist) {
	my ($anchor, $tree) = split('\201', $value);
	print STDERR "found value: <$anchor $tree> for word: $index\n" if ($verbose);
	if ($tc{$tree} > $tree_cutoff) {
	    if (defined $anchor_tbl{$anchor}) { push(@{$anchor_tbl{$anchor}}, $tree); }
	    else { $anchor_tbl{$anchor} = [ $tree ]; }
	}
    }

    foreach $anchor (keys %anchor_tbl) {
	my @treelist = @{$anchor_tbl{$anchor}};
	next if ($#treelist < 0);
	if ($index eq $unknown) {
	    printf F $unknown_fmt, $anchor, join(" ", @treelist);
	} else {
	    printf O $fmt_string, $index, $index, $anchor, join(" ", @treelist);
	}
    }
}
close(F);
close(O);
print STDERR "done.\n";

print STDERR "all done.\n";

