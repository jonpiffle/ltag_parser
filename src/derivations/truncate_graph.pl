#!@PERLBIN@ -w

srand();
while (<>)
{
  chomp;

  # handle features if they exist
  s/ \#/;\#/go;
  s/ \>/\>/go;

  if (/^begin/) { print "$_\n";  next; } 
  if (/^start/) { print "$_\n";  next; } 
  if (/^end/) { print "$_\n";  next; } 

  ($l, $m, $ptrs) = split; 
  if ($ptrs eq '') { 
     print "$l $m\n";
  } else {
     print "$l $m ";
     $ptrs =~ s/\[//go;
     $ptrs =~ s/\]/ /go;
     @plist = split(' ', $ptrs); 
     print '[' , $plist[rand($#plist)] , ']';
     print "\n";
  }
}
