#!/usr/bin/perl -w

$corpusfile='/mnt/linc/xtag2/pkg/wsj88/wsj88.tagged.all.gz';
$morphfile='./morphology/morph_english.db.flat';
$cutoff = 1;

print STDERR "opening $corpusfile...\n";
open(C, "gzip -dc $corpusfile |") or die "cannot open $corpusfile\n";
while (<C>)
{
    chomp;
    my @wds = split;
    foreach (@wds)
    {
	my ($w, $p) = split('_', $_, 2);
	if (defined $seen{$w}) { next; }
	if (defined $unknown{$w}) { $unknown{$w}++; }
	else { $unknown{$w} = 1; }
	if ($unknown{$w} > $cutoff) {
	    undef $unknown{$w};
	    $seen{$w} = 1;
	    #print STDERR "seen: $w\n";
	}
    }
}
close(C);
print STDERR "done\n";

print STDERR "opening $morphfile...\n";
open(M, $morphfile) or die "cannot open $morphfile\n";
while (<M>)
{
    next if /^;;;/;
    my ($ind, $re) = split(' ', $_, 2);
    if (defined $seen{$ind}) {
	print;
    }
}
close(M);
print STDERR "done\n";

