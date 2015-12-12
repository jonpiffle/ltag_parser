#!@PERLBIN@ -w

$start=0;
$allow=1;
while (<>)
{
  chomp;

  # handle features if they exist
  s/ \#/;\#/go;
  s/ \>/\>/go;

  if (/^begin/) { $start=0; $allow=1; print "$_\n";  next; } 
  if (/^start/) { if ($start == 0) { $start=1; print "$_\n";  } else { $allow=0; } next; } 
  if (/^end/) { print "$_\n";  next; } 

  if ($allow) { print "$_\n"; }
}
