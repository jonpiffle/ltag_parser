#!/usr/bin/perl

## Input: etree.out

#  tree_number=1 tree_name=m_anchor@_NP*_$Ins1$ plaus=1
#  (NP (anchor:headp) 
#           (NP_Ins:footp:NA))
#  tree_number=2 tree_name=m_anchor@_ADJP*_$Ins1$ plaus=1
#  (ADJP (NP (anchor:headp)) 
#             (ADJP_Ins:footp:NA))

## Output: tree-args.dat

#  N S A_W1nx0dxN1 NP S NP VP V :ext +NP.
#  V S A_nx0Va1 S NP VP V :ext -NP. +A.

my $verbose = 0;
my $include_num_in_treename = 0;
my $include_ins_in_treename = 0;
my $epsfile = "epsilons.in";
my @emptyelts = ();

sub member
{
    my ($refl, $x) = @_;
    my @l = @{$refl};
    foreach (@l) {
	if ($_ eq $x) { return(1); }
    }
    return(0);
}

sub print_args {
    my ($tree, $treename) = @_;

    print STDERR "tree=$tree\n" if ($verbose);

    $tree =~ s/[\(\)]/ /g;
    $tree =~ s/^\s*//o;
    $tree =~ s/\s*$//o;
    $tree =~ s/^/ /o;
    $tree =~ s/$/ /o;

    ## mark null adj nodes with \201
    $tree =~ s/:NA/\201/go;
    $tree =~ /^\s*([^: ]+)[: ]/;
    my $rootnode = $1;
    $rootnode =~ s/_.*$//o;
    print STDERR "newtree=$tree\n" if ($verbose);

    if (!defined $rootnode) { 
	die "could not find rootnode in: $treename\n$tree\n";
    }

    my $n = '';
    my @treeparts = split(/\s+/, $tree);

    ## get nodes from tree that are not frontier nodes
    my @nodes = grep(!/:/, @treeparts);
    foreach $n (@nodes) {
	## if node is empty or a null adj node or if it's an empty element then skip
	if (($n =~ /\201/) or ($n =~ /^\*/) or (member(\@emptyelts, $n))) {
	    next;
	}
	$n =~ s/^\s*//o; 
	$n =~ s/\s*$//o;
	if ($n =~ /_/) {
	    $n =~ s/_.*$//o;
	}
    }
    print STDERR "nodes=", join(' ', @nodes), "\n" if ($verbose);

    ## get frontier nodes which are marked 
    my @extnodes = grep(/:/, @treeparts);
    print STDERR "extnodes=", join(' ', @extnodes), "\n" if ($verbose);

    my @prnodes = ();
    my $beforeheadp = 1;
    foreach $n (@extnodes) {
	$n =~ s/\201//go;
	$n =~ s/^\s*//o; 
	$n =~ s/\s*$//o;
	if ($n =~ /_/) {
	    $n =~ s/_[^:]+//o;
	}
	if ($n =~ /:headp/) {
	    if ($beforeheadp == 0) {
		print STDERR "duplicate headp nodes in tree: $treename\n";
	    }
	    $beforeheadp = 0;
	    $n =~ /([^:]+):headp/;
	    $anchor = $1;
	} elsif ($n =~ /:footp/) {
	    $n =~ /([^:]+):footp/;
	    $footnode = $1;
	    if ((defined $footnode) and ($footnode ne "")) {
		push(@prnodes, (($beforeheadp) ? '-' : '+') . $footnode . '*');
	    }
	} elsif ($n =~ /:substp/) {
	    $n =~ /([^:]+):substp/;
	    $substnode = $1;
	    if ((defined $substnode) and ($substnode ne "")) {
		push(@prnodes, (($beforeheadp) ? '-' : '+') . $substnode . '.');
	    }
	} else {
	    die "unknown node type: $n in tree: $tree\n";
	}
    }

    if (!defined $anchor) { 
	die "could not find anchor in: $treename\n$tree\n";
    }


    print "$anchor $rootnode $treename";

    if ($#nodes > -1) {
	print join(' ', @nodes);
    }

    if ($#prnodes > -1) {
	print " :ext ", join(' ', @prnodes);
    }
    print "\n";
}

## first read in the empty elements
open(EPS, $epsfile) or die "could not open $epsfile\n";
while (<EPS>) {
    chomp;
    push(@emptyelts, $_);
}
close(EPS);

my $tree = "";
my $treename = "";
while (<>) {
    chomp;
    if (/^\(/) {
	$tree = $_;
    } elsif (/^(tree_number)/) {
	if ($tree ne "") {
	    print_args($tree, $treename);
	}
	$tree = "";
	$treename = "";

	/tree_number=([^ ]*)/;
	$num = $1;
	/tree_name=([^ ]*)/;
	$name = $1;
	/plaus=([^ ]*)/;
	$plaus = $1;

	if ($include_num_in_treename) {
	    $treename = $name . '=' . $num . '_' . $plaus;
	} else {
	    $treename = $name;
	}

	if ($include_ins_in_treename == 0) {
	    $treename =~ s/_\$Ins\d*\$//g;
	}

    } else {
	s/^\s*//o;
	$tree .= " $_";
    }
}

if ($tree ne "") {
    print_args($tree, $treename);
}


