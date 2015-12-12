#!@PERLBIN@ -w

my $verbose = 1;
my $start = 0;
my $allow = 1;

while (<>)
{
  chomp;

  # handle features if they exist
  s/ \#/;\#/go;
  s/ \>/\>/go;

  if (/^begin/) { 

      m/sent=\"([^\"]*)\"/; #" for highlight
      $sent_string = $1;

      if ($verbose) {
	  print STDERR "sent=\"$sent_string\"\n";
      }

      $start = 0; 
      $allow = 1; 

      print "$_\n";  
      next; 
  } 

  if (/^start/) { 

      if ($start == 0) { 
	  $start = 1; 
	  print "$_\n";  
      } else { 
	  $allow = 0; 
      } 
      next; 
  } 

  if (/^end/) { 
      print "$_\n";  
      next; 
  } 

  if ($allow) {

      my ($l, $m, $ptrs) = split; 
      if ($ptrs eq '') { 
	  print "$l $m\n";
      } else {
	  print "$l $m ";
	  $ptrs =~ s/\[//go;
	  $ptrs =~ s/\]/ /go;
	  @plist = split(' ', $ptrs); 
	  print '[' , $plist[0] , ']';
	  print "\n";
      }
  }

}
