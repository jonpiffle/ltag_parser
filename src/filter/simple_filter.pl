#!@PERLBIN@ -w

#$| = 1;
use IO::Handle;
STDOUT->autoflush(1);

my $whpat = 'wh[a-z]*';
my $whtreepat = 'alphaW';
my @delete_pats = qw(
		     alphaG
		     alphaWA
		     Ax1
		     Px1
		     N1
		     nx0nx1ARB
		     nx0ARB
		     nx0A1
		     s0Pnx1
		     alphaI
		     betaI
		     betaNc
		     Vtransn
		     Vintransn
		     alphaInv
		     );

my $prev = "";
my $has_wh = 0;
while (<>)
{
    if ((/^begin/) or (/^end/)) { 
        $has_wh = 0;
	print; 
	next; 
    }
    my $prflag = 1;
    my @line = split;
    if ($line[1] =~ /$whpat/) { $has_wh = 1; }
    if (($has_wh == 0) && (/$whtreepat/)) { $prflag = 0; }
    foreach $pat (@delete_pats) {
	if (/$pat/) {
	    $prflag = 0;
	}
    }
    if ($prflag) { 
	if ($prev ne $_) { print; }
	$prev = $_;
    }
}
