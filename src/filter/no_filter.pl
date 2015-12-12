#!@PERLBIN@

my $prev = "";
while (<>)
{
    if ((/^begin/) or (/^end/)) { 
	print; 
	next; 
    }
    if ($prev ne $_) { print; }
    $prev = $_;
}

