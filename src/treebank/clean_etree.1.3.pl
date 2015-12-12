#!/usr/bin/perl -w

my $rcsid   = '$Id: clean_etree.pl,v 1.3 2001/04/10 18:38:29 anoop Exp $'; #'
my $verbose = 1;
my $print_names = 0;
my $project = shift or die "usage: $0 <project-file>\n";

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
		 $ETREE_INFILE
		 $ETREE_FILE
		 $EPSILONS
		 $ANCHOR_LBL
		 ));

print STDERR "ETREE_FILE=$ETREE_FILE\n"     if ($verbose > 1);
print STDERR "EPSILONS=$EPSILONS\n"         if ($verbose > 1);
print STDERR "ANCHOR_LBL=$ANCHOR_LBL\n"     if ($verbose > 1);
print STDERR "ETREE_INFILE=$ETREE_INFILE\n" if ($verbose > 1);

my $outfile      = $ETREE_FILE;
my $epsfile      = $EPSILONS;
my $anchor_label = $ANCHOR_LBL;
my $filename     = $ETREE_INFILE;

my %ec_tbl = ();

open(O, ">$outfile") or die "could not open $outfile\n";
open(E, ">$epsfile") or die "could not open $epsfile\n";
open(F, $filename) or die "could not find $filename\n";
while (<F>)
{
  if (/^$/) { next; }
  if (/^\#/) {
    chomp;
    s/^\#(\d+):.*$/$1/go;
    print O "tree_number=$_";
    next;
  }
  if ($print_names) {
      if (/^tree_name/) {
	  m/plaus=([^ \t]*)/;
	  my $plaus_value = $1;
	  m/^tree_name=([^ \t]*)/;
	  my $treename = $1;
	  print O " tree_name=$treename plaus=$plaus_value\n";
	  next;
      }
  } else {
      if (/^tree_name/) {
	  print O "\n";
	  next;
      }
  }

  #s/\_unsp/\_u/go; 
  #s/\_next/\_n/go;
  #s/\_mod/\_m/go;
  # unsp, next, mod should just be ignored
  s/\_unsp//go;
  s/\_next//go;
  s/\_mod//go;

  # check if a leaf node is an empty category and store into $epsfile
  my @ecs;
  my $ec;
  @ecs = m/\(([^\(_]*)_e\)/g;
  foreach $ec (@ecs) { 
      if (!defined $ec_tbl{$ec}) {
	  print STDERR "found new empty category: $ec\n" if ($verbose);
	  printf E "%-5s %s\n", $ec, $anchor_label; 
	  $ec_tbl{$ec} = 1;
      }
  }

  s/\_\@/:headp/go;
  s/\_\*/:footp:NA/go;
  s/\_s\@/:substp/go;
  s/\_s/:substp/go;
  s/(:[^_]*)([^\)]*)/$2$1/go;
  s/\$Ins1\$/Ins/go;
  s/\$Ins2\$/Ins/go;
  print O $_;
}
close(F);
close(E);
print STDERR "wrote empty categories to $epsfile\n";
close(O);
print STDERR "wrote clean etrees to $outfile\n";
print STDERR "done.\n";

