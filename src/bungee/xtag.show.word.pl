#!/usr/bin/perl -w

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

my $rcsid = '$Id: xtag.show.word.pl,v 1.5 2000/12/15 15:57:47 anoop Exp $';
my $verbose = 0;
my $showfeats = 1;
my $LEMHOME='@LEMHOME@';

print "$rcsid\n" if ($verbose);
print "verbose level is $verbose\n" if ($verbose);

my $usage="usage: $0 <lang> <word> [<regexp>]\n";
my $lang = shift or die $usage;
my $word = shift or die $usage;
my $nopat = 0;
my $pat = ''; 
$pat = shift or $nopat = 1;
$pat = '.*' if ($nopat);
print STDERR "using pattern: \"$pat\"\n" if ($verbose);
my $i;

my %treeindex = ();
my @treename  = ();
my @trees     = ();
my @treefeat  = ();
my %treetempl = ();
my %treefam   = ();

## read treenames
my $treenames = "$LEMHOME/data/$lang/treenames.dat";
open (F, $treenames) or die "cannot open $treenames\n";
$i = 0;
while (<F>) {
    chomp;
    $treeindex{$_} = $i;
    $treename[$i++] = $_;
}
close(F);
print STDERR "read $i treenames from $treenames\n" 
    if ($verbose);

## read trees
my $trees = "$LEMHOME/data/$lang/xtag.trees.dat";
open (F, $trees) or die "cannot open $trees\n";
$i = 0;
while (<F>) {
    chomp;
    $trees[$i++] = $_;
}
close(F);
print STDERR "read $i trees from $trees\n" 
    if ($verbose);

## read tree features
my $treefeats = "$LEMHOME/data/$lang/treefeats.dat";
if (-f $treefeats) {
  open (F, $treefeats) or die "cannot open $treefeats\n";
  $i = 0;
  while (<F>) {
      chomp;
      $treefeat[$i++] = $_;
  }
  close(F);
  print STDERR "read $i features from $treefeats\n"
      if ($verbose);
} else {
  $showfeats = 0;
}

## read tree features
my $treetempls = "$LEMHOME/data/$lang/feat_templates.dat";
if (-f $treetempls) {
  open (F, $treetempls) or die "cannot open $treetempls\n";
  $i = 0;
  while (<F>) {
      chomp;
      my ($templ, $value) = split('[\s\t]', $_, 2);
      $value =~ s/^\s*//g;
      $value =~ s/\s*$//g;
      $value =~ s/\s*!\s*$//g;
      $treetempl{$templ} = $value;
      $i++;
  }
  close(F);
  print STDERR "read $i templates from $treetempls\n"
      if ($verbose);
} else {
  $showfeats = 0;
}

## read tree families
my $treefams = "$LEMHOME/data/$lang/treefams.dat";
open (F, $treefams) or die "cannot open $treefams\n";
$i = 0;
while (<F>) {
    chomp;
    my ($fam, $num, @treelist) = split;
    $treefam{$fam} = \@treelist;
    $i++;
}
close(F);
print STDERR "read $i families from $treefams\n"
    if ($verbose);

my $pre = $$;
my $predir = '/tmp';
print STDERR "using prefix=$pre\n" if ($verbose);
print STDERR "using temp dir=$predir\n" if ($verbose);
print STDERR "using pattern=$pat\n" if ($verbose);

my $syntax_file = "$LEMHOME/data/$lang/syntax.flat";
my $grep_cmd    = "grep \"INDEX>>$word<<\" $syntax_file\n";

print STDERR "searching for $word in $syntax_file\n";
my $grep_result = `$grep_cmd`;
print STDERR $grep_result if ($verbose > 1);
$grep_result .= "\n";

my @dispnames = ();
my @disptrees = ();
my @dispfeats = ();

my @synentries = split('\n', $grep_result);
my $syn;
foreach $syn (@synentries) {
    $syn =~ y/>/ /s;
    $syn =~ y/</ /s;
    $syn =~ s/^\s*//;
    $syn =~ s/\s*$//;
    my @synlist = split('\s', $syn);

    my $synitem;
    my $index     = '';
    my @poslist   = ();
    my @entrylist = ();
    my @etreelist = ();
    my @featlist  = ();

    while ($synitem = (shift(@synlist))) {
	if ($synitem eq 'INDEX') {
	    $index = shift(@synlist);
	}
	if ($synitem eq 'ENTRY') {
	    push(@entrylist, shift(@synlist));
	    my $poslabel = shift(@synlist);
	    if ($poslabel ne 'POS') {
		die "Error: ENTRY was not followed by POS\n";
	    }
	    push(@poslist, shift(@synlist));
	}
	if ($synitem eq 'TREES') {
	    my $tree = shift(@synlist);
	    if ((defined $tree) && ($tree ne 'FEATURES')) {
		push(@etreelist, $tree);
		unshift(@synlist, 'TREES');
	    }
	    if ((defined $tree) && ($tree eq 'FEATURES')) {
		unshift(@synlist, 'FEATURES');
	    }
	}
	if ($synitem eq 'FAMILY') {
	    my $tree = shift(@synlist);
	    if ((defined $tree) && ($tree ne 'FEATURES')) {
		push(@etreelist, @{$treefam{$tree}});
		unshift(@synlist, 'FAMILY');
	    }
	    if ($tree eq 'FEATURES') {
		unshift(@synlist, 'FEATURES');
	    }
	}
	if ($synitem eq 'FEATURES') {
	    my $feat = shift(@synlist);
	    if (defined $feat) {
		push(@featlist, $feat);
		unshift(@synlist, 'FEATURES');
	    }
	}
    }

    if ($showfeats == 0) { @featlist = (); }

    my $etree;
    foreach $etree (@etreelist) {
	$etree =~ s/^/alpha/;
	$etree =~ s/^/beta/;
	my $treeidx = $treeindex{$etree};
	if (!defined $treeidx) {
	    print STDERR "Warning: Could not find $etree in grammar\n";
	}
	my $disptree = $trees[$treeidx];
	if (!defined $disptree) {
	    print STDERR "Warning: Could not find tree number $treeidx in grammar\n";
	}
	my $anchor;
        foreach $anchor ($disptree =~ /([^: ]+):headp/g) {
	    my $cmpanchor = $anchor;
	    $cmpanchor =~ s/\_//g;
	    my $i;
	    my $sz;
	    $sz = $#entrylist + 1;
	    for ($i = 0; $i < $sz; $i++) {
		if ($poslist[$i] eq $cmpanchor) {
		    $disptree =~ s/\s*$anchor:headp\s*/ ($anchor $entrylist[$i]) /;
		}
	    }
	}
	my $dispname = "$etree\[" . join("+", @entrylist) . "\]";
	my $dispfeat = '';
	$dispfeat = $treefeat[$treeidx] if ($showfeats);
	if (!defined $dispfeat) {
	    print STDERR "Warning: Could not find features for tree number $treeidx in grammar\n";
	}
	$dispfeat =~ s/\#+/\#/g;
	my $feat;
	foreach $feat (@featlist) {
	    my $expfeat = $treetempl{$feat};
	    if (!defined $expfeat) {
		print STDERR "Warning: Could not find $feat in templates\n";
		next;
	    }
	    $expfeat =~ s/\s*,\s*/\#/g;
	    if ($expfeat ne '') {
		$dispfeat .= '\#' . $expfeat;
	    }
	}

        push(@dispnames, $dispname);
        push(@disptrees, $disptree);
        push(@dispfeats, $dispfeat);
    }
}

if ($#dispnames < 0) {
   print STDERR "No matches\n";
   exit(0);
}

my $out_treenames = "$predir/$pre.treenames.dat";
my $out_trees = "$predir/$pre.xtag.trees.dat";
my $out_feats = "$predir/$pre.treefeats.dat";

$SIG{INT} = \&capture_int;

sub capture_int {
    $SIG{INT} = \&capture_int;
    close(TNAMES);
    close(TREES);
    close(FEATS);
    unlink($out_treenames, $out_trees, $out_feats);
    exit(1);
}

open(TNAMES, ">$out_treenames") or die "could not open $out_treenames for output\n";
print STDERR "opened $out_treenames for output\n" if ($verbose);
print TNAMES join("\n", @dispnames);
close(TNAMES);

open(TREES, ">$out_trees") or die "could not open $out_trees for output\n";
print STDERR "opened $out_trees for output\n" if ($verbose);
print TREES join("\n", @disptrees);
close(TREES);

open(FEATS, ">$out_feats") or die "could not open $out_feats for output\n";
print STDERR "opened $out_feats for output\n" if ($verbose);
print FEATS join("\n", @dispfeats);
close(FEATS);

my $runcmd = "$LEMHOME/bin/bungee \"" . $pat . "\" $out_treenames $out_trees $out_feats";
print STDERR "running: $runcmd\n" if ($verbose);
system($runcmd);
unlink($out_treenames, $out_trees, $out_feats);
print STDERR "removed temp files\n" if ($verbose);
1;
