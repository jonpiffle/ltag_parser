#!/usr/bin/perl -w
#
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

my $rcsid = '$Id: printgrammar.pl,v 1.1 2001/04/15 01:58:28 anoop Exp $'; #'

my $progname = $0; $progname =~ s|.*/||;
my $usage = "$progname <grammar>\n";

# retain binary branching of trees 
my $print_binary = 0;

## globals

@label         = ();
@rchild        = ();
@lchild        = ();
#@tree          = ();
@rootnode      = ();
@footp         = ();
@substp        = ();
@headp         = ();
@NA            = ();
#@treename      = ();
$num_trees     = 0;
#$num_treenodes = 0;

die $usage if ($#ARGV < 0);
$grammar = shift(@ARGV);

sub printtree
{   my ($tree) = @_;
    printroot($tree, $rootnode[$tree]);
    print "\n";
}

sub nodelabel
{   my ($tree, $node) = @_;
    my $label = $label[$node];
    my @node_exists;

    $label .= ':footp'  if ((defined $footp[$tree]) and ($footp[$tree] == $node));

    if (defined $substp[$tree]) {
	@node_exists = grep { $_ == $node } @{$substp[$tree]};
    } else {
	@node_exists = ();
    }

    $label .= ':substp' if ($#node_exists == 0);
    $label .= ':headp'  if ((defined $headp[$tree]) and ($headp[$tree] == $node));

    if (defined $NA[$tree]) {
	@node_exists = grep { $_ == $node } @{$NA[$tree]};
    } else {
	@node_exists = ();
    }

    $label .= ':NA' if ($#node_exists == 0);

    return $label;
}

sub printroot
{   my ($tree, $root) = @_;

    if ((!$print_binary) and ($label[$root] eq '@')) {
	printroot($tree, $lchild[$root]);
	printroot($tree, $rchild[$root]);
    } else {
	print '(' . nodelabel($tree, $root);
	if (defined $lchild[$root]) {
	    print ' ';
	    printroot($tree, $lchild[$root]);
	}
	if (defined $rchild[$root]) {
	    print ' ';
	    printroot($tree, $rchild[$root]);
	}
	print ') ';
    }
}

print STDERR "reading grammar: $grammar\n";
my ($user1,$system1,$cuser1,$csystem1) = times;
require $grammar;
my ($user2,$system2,$cuser2,$csystem2) = times;

print STDERR "time taken to read grammar:",
    " user=",    $user2-$user1, 
    " system=",  $system2-$system1, "\n";

for ($i = 0; $i < $num_trees; $i++)
{
    printtree($i);
}

1;



