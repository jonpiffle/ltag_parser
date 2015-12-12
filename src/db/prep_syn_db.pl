#!@PERLBIN@

while (<>) {
   chomp;
   $line = $_;
   $line =~ s/^<<INDEX>>//o;
   $line =~ m/([^<]*)</;
   print $1, '\00', "\n";
   print $_, '\00', "\n";
}

1;
