#!/usr/bin/perl -w
# -*- perl -*-

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

my $rcsid   = '$Id: replace_treelabels.pl,v 1.4 2001/04/10 18:39:18 anoop Exp $'; #'
my $verbose = 0;

print STDERR "$rcsid\n" if ($verbose > 1);

my $rootlabel      = '';
my $treenames_file = '';
my $treenodes_file = '';
my $trees_file     = '';

my %memo_tbl  = ();
my %treenames = ();
my @treenodes = ();
my @rootnode  = ();
my @footnode  = ();

sub initialize_replace_treelabels
{
    my ($project) = @_;

    ## Read the project file if not loaded already
    if (-f $project) {
	require $project;
    } else {
	die "could not load $project\n";
    }

    my $error = "Error: corrupt project file\n";
    die $error if (!defined $GRAMMAR_FILE);
    die $error if (!defined $TREENAMES_TAG);
    die $error if (!defined $TREENODES_TAG);
    die $error if (!defined $TREES_TAG);
    die $error if (!defined $ROOTNODE);

    my $grammar       = $GRAMMAR_FILE;
    my $treenames_tag = $TREENAMES_TAG;
    my $treenodes_tag = $TREENODES_TAG;
    my $trees_tag     = $TREES_TAG;

    ## Set value of root label
    $rootlabel = $ROOTNODE;

    ## Read the filenames from the grammar file
    print STDERR "reading grammar file $grammar\n";
    open(F, $grammar) or die "could not open $grammar\n";
    while (<F>) {
	chomp;
	if (/^$treenames_tag/) {
	    my ($x, $y) = split('=', $_, 2);
	    $treenames_file = $y;
	}
	if (/^$treenodes_tag/) {
	    my ($x, $y) = split('=', $_, 2);
	    $treenodes_file = $y;
	}
	if (/^$trees_tag/) {
	    my ($x, $y) = split('=', $_, 2);
	    $trees_file = $y;
	}
    }
    print STDERR "done.\n";
    close(F);
    read_treenames($treenames_file);
    read_treenodes($treenodes_file);
    read_trees($trees_file);
}

sub read_treenames
{
    my ($treenames_file) = @_;
    my $i = 0;
    print STDERR "reading $treenames_file\n";
    open(F, $treenames_file) or die "cannot open $treenames_file\n";
    while (<F>)
    {
	chomp;
	$treenames{$_} = $i++;
    }
    close(F);
    print STDERR "done.\n";
}

sub read_treenodes
{
    my ($treenodes_file) = @_;
    my $i = 0;
    print STDERR "reading $treenodes_file\n";
    open(F, $treenodes_file) or die "cannot open $treenodes_file\n";
    while (<F>)
    {
	chomp;
	$treenodes[$i++] = $_;
    }
    close(F);
    print STDERR "done.\n";
}

sub read_trees
{
    my ($trees_file) = @_;
    my $i = 0;
    my @line;

    print STDERR "reading $trees_file\n";
    open(F, $trees_file) or die "cannot open $trees_file\n";
    while (<F>)
    {
	chomp;
	@line = split;
	die "empty line in $trees_file: $_\n" 
	    if ((!defined $line[0]) || (!defined $line[1]));
	$rootnode[$i] = $line[0];
	$footnode[$i] = $line[1];
	$i++;
    }
    close(F);
    print STDERR "done.\n";
}

sub find_children
{
    my ($node) = $_[0];
    my $nodedata = $treenodes[$node];
    print STDERR "find_children: node: $node, nodedata: $nodedata\n"
	if ($verbose > 1);
    my @data = split(' ', $nodedata);
    return(traverse_right($data[0]));
}

sub traverse_right
{
    my ($node) = $_[0];
    my $nodedata = $treenodes[$node];
    print STDERR "traverse_right: node: $node, nodedata: $nodedata\n"
	if ($verbose > 1);
    my @data = split(' ', $nodedata);
    my @children = ($node);
    if ($data[2] eq '1') { 
	return(@children); 
    } else { 
	push(@children, traverse_right($data[1])); 
	return(@children); 
    }
}

sub find_treenode
{
    my ($treeindex, $gorn_addr) = @_;

    # check if previously computed
    if (defined $memo_tbl{$treeindex}{$gorn_addr}) {
	print STDERR "previous: treeindex=$treeindex; gorn=$gorn_addr\n" 
	    if ($verbose);
	return($memo_tbl{$treeindex}{$gorn_addr});
    }

    my $root = $rootnode[$treeindex];

    if ($gorn_addr eq $rootlabel) {
	return($root);
    }

    my $curnode = $root;
    my @gornlist = split('_', $gorn_addr);
    my $i;
    my $sz = $#gornlist + 1;
    my @children;

    for ($i = 0; $i < $sz; $i++)
    {
	@children = find_children($curnode);
	if (!defined $children[$gornlist[$i]]) {
	    die "could not resolve $gorn_addr in tree: $treeindex\n";
	}
	$curnode = $children[$gornlist[$i]];
    }

    # store answer in memo table 
    $memo_tbl{$treeindex}{$gorn_addr} = $curnode;
    print STDERR "treeindex=$treeindex; gorn=$gorn_addr; treenode=$curnode\n" 
	if ($verbose);

    return($curnode);
}

sub treename_to_index
{
    my $name = $_[0];
    if (!defined $treenames{$name}) {
	die "could not find treename: <$name>\n";
    }
    return($treenames{$name});
}

## testing
#initialize_replace_treelabels($ARGV[0]);

1;

