#!/usr/bin/perl

# takes a list of tree numbers as input and 
# displays the etree from etree_clean.out

my $LEMHOME = '@LEMHOME@';
my $etreefile = "$LEMHOME/lib/treebank/etree_clean.out";
my $print_treenum = 0;

if ($#ARGV < 0) { die "usage: $0 { <tree-number> }\n"; }

my @numlist = @ARGV;
my $flag = 0;

sub wanted 
{
  my $x = @_[0];
  foreach (@numlist) 
  {
     if ($_ == $x) { return(1); }
  }
  return(0);
}

open (F, $etreefile) or die "cannot find $etreefile\n";
while (<F>) {
  chomp;
  s/^\s+/ /;
  s/\s+$/ /;
  if (/^tree_number/) {
    if ($flag) { 
      $flag = 0; 
      if ($print_treenum) { print " )\n" } 
      else { print "\n";} 
    }
    m/tree_number=(\d+)/;
    if (wanted($1)) { print "( $1 " if ($print_treenum); $flag = 1; }
    next;
  }
  if ($flag) {
    print;
  }
}
close(F);

