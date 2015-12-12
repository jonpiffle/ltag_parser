#!/usr/bin/perl

use IO::Handle;
STDOUT->autoflush(1);

while (<>) {
   chomp;
   if (/^begin/) { print "$_\n"; next; }
   if (/^end/) { print "$_\n"; next; }
   my ($x, $y, $z) = $_;
   (defined $z) or $z = '';
   if (($z ne '') and ($z =~ /^\[/)) {
      print "$x $y $z\n"
   } else {
     $y = $y . $z;
     $y =~ s/ /\201/g;
     print "$x $y\n";
   }
}
1;
