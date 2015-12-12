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

my $rcsid   = '$Id: process_counts.pl,v 1.2 2001/04/10 18:42:06 anoop Exp $'; #'
my $verbose = 0;

print STDERR "$rcsid\n" if ($verbose > 1);

my $model_file = $MODEL_FILE;
%count_tbl = ();

sub store_counts
{
    my ($word, $POS, $tree, $anchor, $treenode, $depword, $depPOS, $deptree) = @_;

    # Initialize
    $count_tbl{$tree}{$deptree}{$treenode} = 0
	if (!defined $count_tbl{$tree}{$deptree}{$treenode});
    $count_tbl{$deptree}{$treenode} = 0
	if (!defined $count_tbl{$deptree}{$treenode});

    # store counts
    $count_tbl{$tree}{$deptree}{$treenode}++;
    $count_tbl{$deptree}{$treenode}++;

}

1;


