#!/usr/bin/perl -w

my $rcsid   = '$Id: clean_etree.pl,v 1.4 2001/04/12 17:38:32 anoop Exp $'; #'

my $usage = "$0 [-e=epsfile] [-a=anchor] [-n] filename\n";

my $verbose = 1;
my $print_names = 0;

my $epsfile      = 'epsilons.in';
my $anchor_label = 'anchor';

if ($#ARGV < 0) { die $usage; }

while ($ARGV[0] =~ /^-/) {
    $epsfile      = $1 if ($ARGV[0] =~ /^-e=(.*)$/);
    $anchor_label = $1 if ($ARGV[0] =~ /^-a=(.*)$/);
    $print_names  = 1  if ($ARGV[0] =~ /^-n$/);
    shift(@ARGV); 
}

print STDERR "using epsfile=$epsfile\n";
print STDERR "using anchor_label=$anchor_label\n";
print STDERR "using print_names=$print_names\n";

my $filename = shift(@ARGV) or die $usage;

my ($user1,$system1,$cuser1,$csystem1) = times;

my %ec_tbl = ();

open(E, ">$epsfile") or die "could not open $epsfile\n";
open(F, $filename) or die "could not find $filename\n";
while (<F>)
{
  if (/^$/) { next; }
  if (/^\#/) {
    chomp;
    s/^\#(\d+):.*$/$1/go;
    print "tree_number=$_";
    next;
  }
  if ($print_names) {
      if (/^tree_name/) {
	  m/plaus=([^ \t]*)/;
	  my $plaus_value = $1;
	  m/^tree_name=([^ \t]*)/;
	  my $treename = $1;
	  print " tree_name=$treename plaus=$plaus_value\n";
	  next;
      }
  } else {
      if (/^tree_name/) {
	  print "\n";
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
  print $_;
}
close(F);
close(E);
print STDERR "wrote empty categories to $epsfile\n";
print STDERR "wrote clean etrees to stdout\n";
print STDERR "done.\n";

my ($user2,$system2,$cuser2,$csystem2) = times;

print STDERR "time:",
    " user=",    $user2-$user1, 
    " system=",  $system2-$system1, "\n";

1;
