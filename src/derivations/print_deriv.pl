#!@PERLBIN@ -w
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

my $rcsid = '$Id: print_deriv.pl,v 1.18 2001/08/22 21:11:50 anoop Exp $';

#### default settings

my $arg = '';
$0 =~ m/.*\/(.*)$/;
my $basename = $1;
my $usage_string = <<EOU;

$basename -- extracts individual derivation trees and derived trees from a forest

usage: $basename [-h] [-p|-d|-b] [-m] [-s] [-e string] [+/-r] [-x|-t] [+/-o] [-v num] [-f] filename ...

  -h          print help
  -p          print parse trees (derived trees) [default]
  -d          print derivation trees
  -b          print both derivation and parse trees (only compatible with -o)
  -m          input forest allows multiple adjunctions at a treenode
  -s          print single best derivation tree in flat format
  -e string   set value for empty_dtr to string [default = "(nil)"]
  -i string   ignore all nodelabels named string [ default = "ignore" ]
  +r          remove artificially inserted nodes [default]
  -r          don't remove any inserted nodes
  -x          accept forest in xtag format [default]
  -t          accept forest in treebank format
  +o          remove duplicates in parse tree output 
  -o          don't remove duplicates in parse tree output [default]
  -v num      set verbosity level (lower the quieter) [default = 1]
  -f          print features 
  filename    input derivation forest

EOU

my $verbose = 0;
print STDERR "$rcsid\n" if ($verbose > 1);

# print derivation or derived trees
my $PRINT_DERIVED = 1;
my $PRINT_DERIVATION = 2;
my $PRINT_BOTH = 3;
my $PRINT_SINGLE = 4;
my $print_option = $PRINT_DERIVED;

# multiple adjunctions allowed at a node
my $multiple_adjunctions = 0;

# print features
my $print_features = 0;

# set to nonzero to remove unwanted inserted nodes 
my $remove_inserted = 1;  

# set to nonzero to remove duplicates in the derived tree output
my $occur_check = 0;

# settings for unwanted inserted nodelabels
my $XTAG_TREES = 1;
my $TREEBANK_TREES = 2;
my $xtag_inserted = '@';
my $treebank_inserted = 'Ins';
my $treebank_anchor = 'anchor';
my $subscript_separator = '_';

my $treetype = $XTAG_TREES;
#my $treetype = $TREEBANK_TREES;

# empty dtr label
my $empty_dtr = '(nil)';
#my $empty_dtr = '0';

# ignore nodelabel 
my $ignore_nodelabel = 'ignore';

my $leaf = '__leaf__';
my $start = 'start';
my $epsilon = 'e';
#my $epsilon = '';

my $nullfoot = '';
my $top = 'top';
my $bot = 'bot';
my $insert_label = '__INSERT_SUBTREE_HERE__';

my $start_num = 0;

my $logprob = 0;     # temp storage for prob in -s option
my @flat_deriv = (); # temp storage for derivation in -s option

#### data structures

my %graph = ();
my %treename = ();
my %nodelabel = ();
my %position = ();
my %postag = ();
my %nodetype = ();
my %inserted = ();
my %feats = ();

my %memo = ();

# nodetypes (used in get_nodetype)
my $UNKNOWN  = 0;
my $EMPTYDTR = 1;
my $INITROOT = 2;
my $AUXROOT  = 3;
my $AUXFOOT  = 4;
my $START    = 5;

my @ndstring = qw(UNKNOWN 
		  EMPTYDTR 
		  INITROOT 
		  AUXROOT 
		  AUXFOOT 
		  START);

sub nodetype_string
{
    my $nt = $_[0];
    return($ndstring[$nt]);
}

sub get_nodetype
{
    my ($node_id, $type) = @_;

    if ($node_id eq $empty_dtr) {
	return($EMPTYDTR);
    }

    if ($node_id =~ /^start/) {
	return($START);
    }

    if ($type eq 'initroot') {
	return($INITROOT);
    }

    if ($type eq 'auxroot') {
	return($AUXROOT);
    }

    if ($type eq 'auxfoot') {
	return($AUXFOOT);
    }

    return($UNKNOWN);
}

sub print_nodeinfo
{
    my $node_id = $_[0];
    print STDERR "node_id:- $node_id\n";
    print STDERR "  nodetype  {", nodetype_string($nodetype{$node_id}), "}\n";
    print STDERR "  dtrs      {", $graph{$node_id}, "}\n";
    if ($node_id =~ /^start/) { return; }
    print STDERR "  treename  {", $treename{$node_id}, "}\n";
    print STDERR "  nodelabel {", $nodelabel{$node_id}, "}\n";
    print STDERR "  position  {", $position{$node_id}, "}\n";
}

sub get_word_from_treename
{
    my $treename = $_[0];
    my $w = '';
    $treename =~ m/\[([^\]]*)\]/;
    $w = $1;
    return(($w eq 'nil') ? $epsilon : $w);
}

sub compute_derived
{
    my $i;
    my @all = ();

    $nodetype{$empty_dtr} = $EMPTYDTR;
    if ($start_num == 0) { return([]); }
    for ($i = 1; $i <= $start_num; $i++)
    {
	my $node_id = $start . '_' . $i;
	my $result = compute_derived_recursive($node_id, 0);
        $memo{$node_id} = $result;
	push(@all, $result);
    }
    return(\@all);
}

sub compute_derived_recursive
{
    my ($node_id, $mem) = @_;
    my $node_type;
    my @dtr_ORlist;

    $node_type = $nodetype{$node_id};

    if (!defined $node_type)
    {
	die "Error: unknown node_type: $node_type for node_id: $node_id\n";
    }

    if ($node_type eq $EMPTYDTR) {
	return([]);
    }

    if (defined($memo{$node_id})) {
	my $result = $memo{$node_id};
	my $new = $mem . ':' . $insert_label;
	foreach (@{$result}) { 
	    s/\w+:$insert_label/$new/go; 
	}
	return($result);
    }

    if (!defined($graph{$node_id})) {
	die "hey! could not find $node_id in graph table\n";
    }

    # start symbols are added on artificially
    # they don't need to be part of the output
    if ($node_id =~ /^start/) {
	return(compute_derived_recursive($graph{$node_id}, $mem));
    }

    @dtr_ORlist = split(' ', $graph{$node_id});

    if ($#dtr_ORlist < 0) {
	die "hey! found no daughters for node: $node_id\n";
    }

    if ($node_type == $AUXFOOT) {
	return([$mem . ':' . $insert_label]);
    }

    # check terminal symbols at leaf nodes
    # make sure that the leaf node is not a footnode
    if (($dtr_ORlist[0] eq $leaf) && 
	($node_type != $AUXFOOT)) 
    {
	if (($print_features) && (defined $feats{$node_id})) {
	    return([get_word_from_treename($treename{$node_id}) . 
		    "\202" . $feats{$node_id}]);
	} else {
	    return([get_word_from_treename($treename{$node_id})]);
	}
    }

    # traverse daughters:
    my @clist = ();
    foreach $ANDpair (@dtr_ORlist) {
	my ($x, $y) = split(',', $ANDpair);
	my $result = get_ANDpair_results($node_id, $node_type, $x, $y, $mem);
	print STDERR "begin result:\n", join("\n", @{$result}), "\nend\n"
	    if ($verbose > 1);
	foreach(@{$result}) { push(@clist, $_); }
    }

    # return current ORlist:
    $memo{$node_id} = \@clist;
    return(\@clist);
}

sub get_ANDpair_results
{
    my ($node_id, $node_type, $x, $y, $mem) = @_;

    my $x_type = $nodetype{$x};
    my $y_type = $nodetype{$y};
    my @result = ();

    if (($x_type == $AUXROOT) && ($y_type == $AUXROOT)) {
	die "Error: both dtrs cannot be auxiliary trees for node: $node_id\n";
    }

    my $x_result = compute_derived_recursive($x, (($x_type == $AUXROOT) ? $node_id : $mem));
    my $y_result = compute_derived_recursive($y, (($y_type == $AUXROOT) ? $node_id : $mem));

    if ($verbose > 1) {
	print STDERR "x_type: ", nodetype_string($x_type), "\n";
	print STDERR "x_result: $x\n", join("\n", @{$x_result}), "\n";
	print STDERR "y_type: ", nodetype_string($y_type), "\n";
	print STDERR "y_result: $y\n", join("\n", @{$y_result}), "\n";
    }

    # substitution
    if ($x_type == $INITROOT)
    {
	if ($verbose > 1) {
	    print STDERR "node_id: $node_id:  substitution\n";
	    print_nodeinfo($node_id);
	}
	foreach (@{$x_result}) { push(@result, $_); }
    }

    elsif ($y_type == $INITROOT)
    {
	if ($verbose > 1) {
	    print STDERR "node_id: $node_id:  substitution\n";
	    print_nodeinfo($node_id);
	}
	foreach (@{$y_result}) { push(@result, $_); } 
    }

    # adjunction
    elsif ($x_type == $AUXROOT)
    {
	if ($verbose > 1) {
	    print STDERR "node_id: $node_id:  adjunction\n";
	    print_nodeinfo($node_id);
	}

	# push_underfoot($rootnode, $underfoot, $auxroot)
	my $auxresult = push_underfoot($node_id, $y_result, $x_result);
	foreach(@{$auxresult}) { push(@result, $_); }
    }

    elsif ($y_type == $AUXROOT)
    {
	if ($verbose > 1) {
	    print STDERR "node_id: $node_id:  adjunction\n";
	    print_nodeinfo($node_id);
	}

	# push_underfoot($rootnode, $underfoot, $auxroot)
	my $auxresult = push_underfoot($node_id, $x_result, $y_result);
	foreach(@{$auxresult}) { push(@result, $_); }
    }

    # completion: internal branch
    elsif ($position{$node_id} eq $bot)
    {
	# combine x_result and y_result and return
	# for future combination with node.top

	if ($verbose > 1) {
	    print STDERR "node_id: $node_id:  completion 1, internal binary branch\n";
	    print_nodeinfo($node_id);
	}

	my ($x, $y);
	if ($x_type == $EMPTYDTR) { 
	    foreach (@{$y_result}) { 
		push(@result, "$_"); 
	    }
	} elsif ($y_type == $EMPTYDTR) {
	    foreach (@{$x_result}) { 
		push(@result, "$_"); 
	    }
	} else {
	    foreach $x (@{$x_result}) {
		foreach $y (@{$y_result}) {
		    push(@result, "$x $y");
		}
	    }
	}
    }

    # completion: internal top
    elsif ($position{$node_id} eq $top)
    {
	if ($verbose > 1) {
	    print STDERR "node_id: $node_id:  completion 2, internal unary branch\n";
	    print STDERR "$y: ", nodetype_string($y_type), "\n";
	    print_nodeinfo($node_id);
	}

	if ($x_type == $EMPTYDTR) { 
	    foreach (@{$y_result}) { 
		push(@result, subtree_when_inserted($node_id, $_));
	    }
	} elsif ($y_type == $EMPTYDTR) {
	    foreach (@{$x_result}) { 
		push(@result, subtree_when_inserted($node_id, $_));
	    }
	} else {
	    die "Error: node_id: $node_id has two nonempty daughters\n";
	}
    }

    else {
	print STDERR "Error: should not reach here\n";

	if ($verbose > 1) {
	    print STDERR "node_id: $node_id:  curious phenomena\n";
	    print STDERR "$x: ", nodetype_string($x_type), "\n";
	    print STDERR "$y: ", nodetype_string($y_type), "\n";
	    print_nodeinfo($node_id);
	}
    }

    # occur check
    if ($occur_check) {
	my @result1 = ();
	foreach (@result) {
	    if (!is_member($_, @result1)) { push(@result1, $_); }
	}
	return(\@result1);
    } else {
	return(\@result);
    }
}

sub is_member
{
  my ($x, @l) = @_;
  foreach (@l) {
   if ($_ eq $x) { return(1); }
 }
 return(0);
}

sub push_underfoot
{
    my ($node_id, $underfoot, $auxroot) = @_;
    my ($uf, $ar);
    my @result = ();
    foreach $uf (@{$underfoot}) {
	foreach $ar (@{$auxroot}) {
	    my $myar = $ar;
	    my $arpat = $node_id . ':' . $insert_label;
	    if ($myar =~ /$arpat/) {
		$myar =~ s/$arpat/$uf/;
		push(@result, $myar);
	    } else {
		die "Error: could not find footnode: $arpat in auxtree: $ar\n";
	    }
	}
    }
    return(\@result);
}

sub subtree_when_inserted
{
    my ($node_id, $subtree) = @_;
    my $newsubtree = '';
    my $fmt = '';

    if (($multiple_adjunctions && ($nodetype{$node_id} == $AUXROOT)) ||
	(($remove_inserted) && (defined($inserted{$node_id})))) {
	$fmt = "%s";
    } else {
	$fmt = "( $nodelabel{$node_id} %s )";
    }
    return(sprintf($fmt, $subtree));
}

sub is_inserted
{
    my $nodelabel = $_[0];
    if ($treetype == $XTAG_TREES) {
	if ($nodelabel eq $xtag_inserted) { 
	    return(1); 
	}
    } elsif ($treetype == $TREEBANK_TREES) {
	if ($nodelabel =~ /\Q$xtag_inserted\E/) { 
	    return(1); 
        } elsif ($nodelabel =~ /\Q$treebank_anchor\E/) {
            return(1);
	} else {
	    my ($nl, $s) = split($subscript_separator, $nodelabel, 2);
	    if ((defined $s) && ($s eq $treebank_inserted)) {
		return(1);
	    }
	}
    } else {
	print STDERR "Error: treetype=$treetype not implemented in is_inserted\n";
    }
    if ($nodelabel eq $ignore_nodelabel) {
        return(1);
    }
    return(0);
}

sub compute_derivation
{
    my $i;
    my @all = ();

    $nodetype{$empty_dtr} = $EMPTYDTR;
    if ($start_num == 0) { return([]); }
    for ($i = 1; $i <= $start_num; $i++)
    {
	my $node_id = $start . '_' . $i;
	my $result = compute_derivation_recursive($node_id, '');
        $memo{$node_id} = $result;
	push(@all, $result);
    }
    return(\@all);
}

sub compute_derivation_recursive
{
    my ($node_id, $mem) = @_;
    my $node_label;
    my $node_type;
    my @dtr_ORlist;

    $node_type = $nodetype{$node_id};

    if (!defined $node_type)
    {
	die "Error: unknown node_type: $node_type for node_id: $node_id\n";
    }

    if ($node_type eq $EMPTYDTR) {
	return([]);
    }

    if (defined($memo{$node_id})) {

	if (($mem) && 
	    ($position{$node_id} eq $top) &&
	    (($node_type == $AUXROOT) || 
	     ($node_type == $INITROOT)))
	{

	    my $result = $memo{$node_id};
	    my $new = '<' . $nodelabel{$mem} . '>';
	    foreach (@{$result}) { 
		s/<[^>]*>/$new/o; 
	    }
	    return($result);

	} else {

	    return($memo{$node_id});

	}
    }

    if (!defined($graph{$node_id})) {
	die "hey! could not find $node_id in graph table\n";
    }

    # start symbols are added on artificially
    # they don't need to be part of the output
    if ($node_id =~ /^start/) {
	return(compute_derivation_recursive($graph{$node_id}, $mem));
    }

    @dtr_ORlist = split(' ', $graph{$node_id});

    if ($#dtr_ORlist < 0) {
	die "hey! found no daughters for node: $node_id\n";
    }

    if ($dtr_ORlist[0] eq $leaf) {
	return([]);
    }

    if (($position{$node_id} eq $top) &&
	(($node_type == $AUXROOT) || 
	 ($node_type == $INITROOT)))
    {
	$node_label = $treename{$node_id};
	if ($verbose > 1) {
	    print STDERR "Treename: $node_label\n";
	}
	if ($mem) {
	    $node_label .= '<' . $nodelabel{$mem} . '>';
	}
    } else {
	$node_label = '';
    }

    # traverse daughters:
    my @clist = ();
    foreach $ANDpair (@dtr_ORlist) {
	my ($x, $y) = split(',', $ANDpair);

	my $x_type = $nodetype{$x};
	my $y_type = $nodetype{$y};

	my $x_result = compute_derivation_recursive($x, $node_id);
	my $y_result = compute_derivation_recursive($y, $node_id);

	my @x_list = @{$x_result};
	my @y_list = @{$y_result};

	if (($#x_list < 0) && ($#y_list < 0)) {
	    if ($node_label) {
		push(@clist, "$node_label");
	    }
	} elsif ($#x_list < 0) {
	    foreach (@y_list) { 
		if ($node_label) {
		    push(@clist, "( $node_label $_ )"); 
		} else {
		    push(@clist, "$_");
		}
	    }
	} elsif ($#y_list < 0) {
	    foreach (@x_list) { 
		if ($node_label) {
		    push(@clist, "( $node_label $_ )"); 
		} else {
		    push(@clist, "$_");
		}
	    }
	} else {
	    foreach $x (@x_list) {
		foreach $y (@y_list) {
		    if ($node_label) {
			push(@clist, "( $node_label $x $y )");
		    } else {
			push(@clist, "$x $y");
		    }
		}
	    }
	}
	print STDERR "begin clist:\n", join("\n", @clist), "\nend\n"
	    if ($verbose > 1);
    }

    # return current ORlist:
    $memo{$node_id} = \@clist;
    return(\@clist);
}

sub compute_flat_derivation
{
    my $i;

    $nodetype{$empty_dtr} = $EMPTYDTR;
    if ($start_num == 0) { return(); }
    for ($i = 1; $i <= $start_num; $i++)
    {
	my $node_id = $start . '_' . $i;
	compute_flat_recursive($node_id, '');
    }
}

sub compute_flat_recursive
{
    my ($node_id, $mem) = @_;
    my $node_type;
    my @dtr_ORlist;
    my $new_mem;

    $node_type = $nodetype{$node_id};

    if (!defined $node_type)
    {
	die "Error: unknown node_type: $node_type for node_id: $node_id\n";
    }

    if ($node_type eq $EMPTYDTR) {
	return();
    }

    if (!defined($graph{$node_id})) {
	die "hey! could not find $node_id in graph table\n";
    }

    # start symbols are added on artificially
    # they don't need to be part of the output
    if ($node_id =~ /^start/) {
	compute_flat_recursive($graph{$node_id}, $mem);
	return;
    }

    @dtr_ORlist = split(' ', $graph{$node_id});

    if ($#dtr_ORlist < 0) {
	die "hey! found no daughters for node: $node_id\n";
    }

    if ($dtr_ORlist[0] eq $leaf) {
	return([]);
    }

    my ($word_label, $word_index, $word_node);
    my ($word_pos, $prob, $tree_label, $anchor_label);
    my ($parent_label, $parent_pos, $parent_node);
    my ($parent_index, $parent_prob, $ptree_label, $panchor_label);
    
    if (($position{$node_id} eq $top) &&
	(($node_type == $AUXROOT) || 
	 ($node_type == $INITROOT)))
    {
	$new_mem = $node_id;

	$word_label = $treename{$node_id};
	$word_pos = $postag{$node_id};
	$word_node = $nodelabel{$node_id};
	($word_index, $prob, $tree_label) = split('\/\/', $word_label, 3);

	if ($mem eq '') { $logprob = $prob; }
	
	$tree_label =~ m/\[([^\]]*)\]/;
	$word_label = $1;
	$tree_label =~ s/\[[^\]]*\]//;

	if ($tree_label =~ /anchor/) {
	    $anchor_label = 'anchor'; 
	} else {
	    $anchor_label = $word_pos;
	}

	if ($mem) {
	    $parent_label = $treename{$mem};
	    $parent_pos = $postag{$mem};
	    $parent_node = $nodelabel{$mem};
	    ($parent_index, $parent_prob, $ptree_label) = split('\/\/', $parent_label, 3);

	    $ptree_label =~ m/\[([^\]]*)\]/;
	    $parent_word = $1;
	    $ptree_label =~ s/\[[^\]]*\]//;

	    if ($ptree_label =~ /anchor/) {
		$panchor_label = 'anchor'; 
	    } else {
		$panchor_label = $parent_pos;
	    }

	} else {
	    $parent_word = '**TOP**';
	    $parent_node = '-';
	    $parent_pos = '-';
	    $parent_index = '-';
	    $parent_prob = '-';
	    $ptree_label = '-';
	    $panchor_label = '-';
	}

	$flat_deriv[$word_index]  = 
	    "$word_label   $word_pos   $tree_label  $anchor_label  1  ";
	$flat_deriv[$word_index] .= 
	    "$parent_node  1  $parent_word  $parent_pos  $ptree_label  $panchor_label";

    } else {

	$new_mem = $mem;
    }

    # traverse daughters:
    $ANDpair = shift(@dtr_ORlist);
    my ($x, $y) = split(',', $ANDpair);

    compute_flat_recursive($x, $new_mem);
    compute_flat_recursive($y, $new_mem);
    return;
}

my @files = ();

while ($arg = shift(@ARGV)) {
    if (($arg =~ /^-h/) || ($arg =~ /^-\?/)) {
	print STDERR $usage_string;
	exit(1);
    } elsif ($arg =~ /^-p/) {
	$print_option = $PRINT_DERIVED;
    } elsif ($arg =~ /^-d/) {
	$print_option = $PRINT_DERIVATION;
    } elsif ($arg =~ /^-b/) {
	$print_option = $PRINT_BOTH;
    } elsif ($arg =~ /^-m/) {
	$multiple_adjunctions = 1;
    } elsif ($arg =~ /^-s/) {
	$print_option = $PRINT_SINGLE;
    } elsif ($arg =~ /^-e/) {
	my $e = shift(@ARGV);
	$empty_dtr = $e;
    } elsif ($arg =~ /^-i/) {
	my $i = shift(@ARGV);
	$ignore_nodelabel = $i;
    } elsif ($arg =~ /^\+r/) {
	$remove_inserted = 1;
    } elsif ($arg =~ /^-r/) {
	$remove_inserted = 0;
    } elsif ($arg =~ /^-x/) {
	$treetype = $XTAG_TREES;
    } elsif ($arg =~ /^-t/) {
	$treetype = $TREEBANK_TREES;
    } elsif ($arg =~ /^\+o/) {
	$occur_check = 1;
    } elsif ($arg =~ /^-o/) {
	$occur_check = 0;
    } elsif ($arg =~ /^-v/) {
	my $num = shift(@ARGV);
	$verbose = $num;
    } elsif ($arg =~ /^-f/) {
	$print_features = 1;
    } elsif ($arg =~ /^-/) {
	print STDERR "unknown option: $arg\n";
	print STDERR $usage_string;
	exit(-1);
    } else {
	push(@files, $arg);
    }
}

if ($print_option == $PRINT_BOTH) {
    $occur_check = 0;
}

if ($treetype == $TREEBANK_TREES) {
    $epsilon = '(-NONE- 0)';
}

if ($#files == -1) {
    push(@files, "-");
}

my $total = 0;
foreach $filename (@files) {
    if ($filename eq "-") {
	*F = STDIN;
    } else {
	open(F, $filename) or die "cannot open $filename\n";
    }
    while (<F>)
    {
	my ($result, $all, $cnt);
	my ($treename, $nodepos, $postag, $type, $rootfoot, $feats);

	chomp;
	if (/^begin/) {

	    m/sent=\"([^\"]*)\"/; #" for highlight
	    $sent_string = $1;

	    if ($verbose) {
		print STDERR "sent=\"$sent_string\"\n";
	    }

	} elsif (/^end/) {

	    if ($verbose) {
		print STDERR "numNodes=" , scalar (keys %graph), "\n"; 
	    }

	    $cnt = 0;
	    if ($print_option == $PRINT_DERIVED) { 
		$all = compute_derived(); 
		foreach $result (@{$all}) { 
		    print join("\n", @{$result}), "\n";
		    $cnt += (scalar @{$result});
		}
	    } elsif ($print_option == $PRINT_DERIVATION) { 
		$all = compute_derivation(); 
		foreach $result (@{$all}) {
		    print join("\n", @{$result}), "\n";
		    $cnt += (scalar @{$result});
		}
	    } elsif ($print_option == $PRINT_BOTH) {
		$all = compute_derivation();
		%memo = ();
		my $all2 = compute_derived();
		my @all_list = @{$all};
		my @all2_list = @{$all2};
		my $sz = $#all_list + 1;
		my ($i, $j);
		for ($i = 0; $i < $sz; $i++) {
		    my @result1 = @{$all_list[$i]};
		    my @result2 = @{$all2_list[$i]};
		    my $sz2 = $#result1 + 1;
		    for ($j = 0; $j < $sz2; $j++) {
			print $result1[$j], "\n";
			print $result2[$j], "\n";
		    }
		    $cnt += $sz2 - 1;
		}
	    } elsif ($print_option == $PRINT_SINGLE) { 
		$logprob = 0;
		@flat_deriv = ();
		print "\n";
		print "sent=\"$sent_string\"\n";
		compute_flat_derivation(); 
		print "prob=$logprob\n";
		my ($i, $sz);
		$sz = $#flat_deriv + 1;
		for ($i = 0; $i < $sz; $i++)
		{
		    print "$i ", $flat_deriv[$i], "\n";
		}
		print "\n";
	    } else {
		print STDERR "Error: unknown print option: $print_option\n";
		exit(-1);
	    }

	    if ($verbose) {
		print STDERR "count=$cnt\n";
		print STDERR "ptr=$total\n";
	    }
	    $total += $cnt;

	    # reset storage for next graph
	    %graph = (); 
	    %treename = ();
	    %nodelabel = ();
	    %position = ();
	    %postag = ();
	    %nodetype = ();
	    %inserted = ();
	    %feats = ();
	    %memo = ();  

	    $start_num = 0;

	} else {

	    # handle features if they exist
	    s/ \#/;\#/go;
	    s/ \>/\>/go;

	    my ($node_id, $label, $dtrs) = split;
	    $node_id =~ s/:$//o;

	    if ($node_id =~ /^start$/) { 

		$node_id = $node_id . '_' . ++$start_num; 
		$graph{$node_id} = $label;

	    } else {

		if ((!defined $dtrs) || ($dtrs eq '')) { 
		    $dtrs = $leaf; 
		}

		$label =~ s/>\[(.*)\]$/>/o;  # remove all features
		if ((defined $1) && ($1)) {
		    $feats = $1;
		    $feats =~ s/^<>$//g;
		    #$feats =~ s/\201/ /g;
		} else {
		    $feats = '';
		}

		$label =~ s/></ /go;
		$label =~ s/</ /o;
		$label =~ s/>//o;

		($treename, $nodepos, $postag, $type, $rootfoot) = 
		    split(' ', $label, 5);
		my ($n, $p) = split('/', $nodepos);

		if ($dtrs ne $leaf) {
		    $dtrs =~ s/\[//go;
		    $dtrs =~ s/\]/ /go;
		    $dtrs =~ s/\s*$//o;
		}

		if ($print_option != $PRINT_SINGLE) {
		    $treename =~ s/.*\/\///g;
		}

		$treename{$node_id}  = $treename;
		$nodelabel{$node_id} = $n;
		$position{$node_id}  = $p;
		$postag{$node_id}  = $postag;
		$graph{$node_id}     = $dtrs;
		if ($feats) { $feats{$node_id} = $feats; }
		if (is_inserted($n)) { $inserted{$node_id} = 1; }
	    }

	    $nodetype{$node_id}  = get_nodetype($node_id, $type);
	    print_nodeinfo($node_id) if ($verbose > 1);
	}
    }
    close(F);
}

if ($verbose) {
    print STDERR "final total=$total\n";
}

1;


