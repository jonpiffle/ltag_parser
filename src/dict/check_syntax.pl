#!@PERLBIN@ -w
#
# check_syntax.pl
#
# checks the consistency of the syntax flat file. flags an error if a
# tree/family is used in the syntax which does not exist in the
# grammar

my $LEMHOME = '@LEMHOME@';
my $XTAG_ALPHA = '';
my $XTAG_BETA  = '';

if (!defined $LEMHOME) { die "install error\n"; }

my $usage = "usage: $0 <lang>\n";
my $LANG = shift or die $usage;

# assumes usual filenames for treenames and treefams

$treenames_file = "$LEMHOME/data/$LANG/treenames.dat";
$treefams_file  = "$LEMHOME/data/$LANG/treefams.dat";
$syntax_file    = "$LEMHOME/data/$LANG/syntax.flat";

if (!(-f $treenames_file)) {
    die "could not find $treenames_file\n";
}
if (!(-f $treefams_file)) {
    die "could not find $treefams_file\n";
}
if (!(-f $syntax_file)) {
    die "could not find $syntax_file\n";
}

my %treenames = ();
my $i = 0;
open(F, $treenames_file) or die "could not open $treenames_file\n";
while (<F>) {
    chomp;
    $treenames{$_} = 1;
    $i++;
}
close(F);
print STDERR "read $i treenames from $treenames_file\n";

my %treefams = ();
$i = 0;
open(F, $treefams_file) or die "could not open $treefams_file\n";
while (<F>) {
    chomp;
    my @tlist = split;
    $treefams{$tlist[0]} = $#tlist - 1;
    $i++;
}
close(F);
print STDERR "read $i tree families from $treefams_file\n";

my %missing = ();
my $total_lextrees = 0;
open(F, $syntax_file) or die "could not open $syntax_file\n";
while (<F>) {
    chomp;
    s/^.*<<TREES>>//g;
    s/^.*<<FAMILY>>//g;
    s/<<FEATURES>>.*$//g;
    my @tlist = split;
    my $t;
    foreach $t (@tlist) {
	$t =~ s/^$XTAG_ALPHA/alpha/;
	$t =~ s/^$XTAG_BETA/beta/;
        if (defined $treefams{$t}) {
            $total_lextrees += $treefams{$t};
        } elsif (defined $treenames{$t}) {
            $total_lextrees++;
        } 
	next if ((defined $treenames{$t}) or (defined $treefams{$t}));
	if (!defined $missing{$t}) {
	    $missing{$t} = 1;
	    print "Error: $t not available in $LANG grammar\n";
	}
    }
}
close(F);
print STDERR "Total number of lexicalized trees=$total_lextrees\n";

