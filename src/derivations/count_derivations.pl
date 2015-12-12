#!@PERLBIN@
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

my $rcsid = '$Id: count_derivations.pl,v 1.5 2000/11/30 19:02:21 anoop Exp $';

my $base = $0;
$base =~ s/.*\///g;
my $usage_string = <<EOU;

$base -- counts number of derivations in a forest

usage: $base [-v] [-h] filename ...

    -v       verbose output
    -h       print usage
   filename  forest output from the parser

EOU

my @files = ();
my $verbose = 0;
print STDERR "$rcsid\n" if ($verbose > 1);

while ($arg = shift(@ARGV)) {
    if (($arg =~ /^-h/) || ($arg =~ /^-\?/)) {
        print $usage_string;
        exit(1);
    } elsif ($arg =~ /^-v/) {
        $verbose = 1;
    } elsif ($arg =~ /^-/) {
        print "unknown option: $arg\n";
        print $usage_string;
        exit(-1);
    } else {
        push(@files, $arg);
    }
}

if (!defined $opt) { $verbose = 0; }
if ($opt eq '-v') { $verbose = 1; }
if ($opt eq '-h') { print $usage; exit(0); }

my %graph = ();
my %memo = ();
my $empty_dtr = '(nil)';
my $leaf = '__leaf__';
my $start = 'start';
my $start_num = 0;

if ($#files == -1) {
    push(@files, "-");
}

foreach $filename (@files) {
    if ($filename eq "-") {
        *F = STDIN;
    } else {
        open(F, $filename) or die "cannot open $filename\n";
    }
    while (<F>)
    {
	my $cnt = 0;

	chomp;

	if (/^begin/) {

	    m/sent=\"([^\"]*)\"/; 
	    $sent_string = $1;
	    print "sent=\"$sent_string\"\n";

	} elsif (/^end/) {

	    print "numNodes=" , scalar (keys %graph), "\n" if ($verbose); 

	    $cnt = count_derivations();

	    print "numMemoized=" , scalar (keys %memo), "\n" if ($verbose); 
	    print "count=$cnt\n";

	    %graph = (); # reset storage for next graph
	    %memo = ();  # reset memoization table for next graph
	    $start_num = 0;

	} else {

	    # handle features if they exist
	    s/ \#/:\#/go;
	    s/ \>/\>/go;

	    ($node_id, $label, $dtrs) = split;
	    $node_id =~ s/:$//o;
	    if ($node_id =~ /^start$/) { $node_id = $node_id . '_' . ++$start_num; }
	    elsif ($dtrs eq "") { $dtrs = $leaf; }
	    $graph{$node_id} = $label . ' ' . $dtrs;
	}
    }
}

sub count_derivations
{
    my $i;
    my $cnt = 0;

    if ($start_num == 0) { return(0); }
    for ($i = 1; $i <= $start_num; $i++)
    {
	my $node_id = $start . '_' . $i;
	$cnt += count_derivations_recursive($node_id);    #  - add OR nodes
    }
    return($cnt);
}

# usage: count_derivations_recursive($node_id)
# - returns total number of derivations for graph rooted by $node_id
# - uses memoization to remember if value for $node_id has been 
#   computed before
#
sub count_derivations_recursive
{
    my $node_id = @_[0];
    my $lab, $dtrs;
    my $ORcnt, $ANDcnt;
    my @dtr_ORlist;

    if ($node_id eq $empty_dtr) {
	return(1); 
    }

    # check if we have computed value for $node_id before
    if (defined($memo{$node_id})) {
	return($memo{$node_id});
    }

    if (!defined($graph{$node_id})) {
	die "hey! could not find $node_id in graph table\n";
    }

    ($lab, $dtrs) = split(' ', $graph{$node_id});

    if ($dtrs eq $leaf) {
	return(1); 
    }

    if ($node_id =~ /^start/) { 
	return(count_derivations_recursive($lab)); 
    }

    if ($dtrs eq "") {
	die "hey! found no daughters for node: $node_id\n";
    }

    $dtrs =~ s/\[//go;
    @dtr_ORlist = split(/\]/, $dtrs);

    # traverse daughters:
    #  - multiply AND nodes
    #  - add OR nodes

    $ORcnt = 0;

    foreach $ANDpair (@dtr_ORlist) {
	my ($x, $y) = split(',', $ANDpair);
	my $xcnt = count_derivations_recursive($x);
	my $ycnt = count_derivations_recursive($y);
	$ANDcnt = $xcnt * $ycnt;
	$ORcnt += $ANDcnt;
    }

    $memo{$node_id} = $ORcnt;
    return($ORcnt);
}

