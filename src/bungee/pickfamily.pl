#!@PERLBIN@

die "usage: pickfamily family-name family-data treename-data\n" if ($#ARGV < 2);
$fam = shift(@ARGV);
$famdata = shift(@ARGV);
$treedata = shift(@ARGV);

open(F, $famdata) || die "could not open $famdata\n";
while (<F>) {
    chomp;
    ($name, $num, $treenames) = split(/\s/, $_, 3);
    if ($name =~ /$fam/) {
	foreach (split(/\s/, $treenames)) {
	    $idx{$_} = 1;
	}
    }
}
close(F);

open (T, $treedata) || die "could not open $treedata\n";
while(<T>) {
    chomp;
    if ($idx{$_} == 1) {
	print $_, "\n";
    } else {
	print "NOMATCH\n";
    }
}
close(T);

