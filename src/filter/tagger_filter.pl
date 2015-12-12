#!@PERLBIN@ -I@LEMHOME@/src/filter -w

use IPC::Open2;

require "pos_mapping.ph" 
    or die "Installation botched. Could not find pos_mapping.ph\n";
defined(%posmapping)
    or die "Installation botched. Table pos_mapping not defined\n";

#$| = 1;
use IO::Handle;
STDOUT->autoflush(1);

my @default_entries = ("%d %s %s N alphaNXN -12.00\n", 
		       "%d %s %s N alphaN -12.00\n",
		       "%d %s %s N betaNn -12.00\n");

my $tagger_file = shift;
my $run_tagger = 1;

if (defined $tagger_file) {
    $run_tagger = 0;
}

my $verbose = 0;
my $tagger_pgm = '@TAGGER@';
my $tagger_delim = '_';
my $whpat = 'wh[a-z]*';
my $whtreepat = 'alphaW';
my @delete_pats = qw(
		     alphaG
		     alphaWA
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

my $prev = '';
my $has_wh = 0;
my $ln = 0;
my @sent = ();
my @postags = ();
my $runprogram = $tagger_pgm;
my @tagged_file = ();

if ($run_tagger) {
    open2(*README, *WRITEME, $runprogram) or die "could not open2 $runprogram\n";
} else {
    open(F, $tagger_file) or die "could not open $tagger_file\n";
    my $i = 0;
    while (<F>) {
	chomp;
	$tagged_file[$i++] = $_;
    }
    close(F);
}

my $i = 0;
while (<>)
{
    chomp;
    $ln++;
    if ((/^begin/) or (/^end/)) { 
        $has_wh = 0;
	if (/^begin/) {
	    @postags = ();
	    /^begin sent=\"([^\"]*)\"/;
	    my $sentence = $1;
	    die "Error: no sentence found at line $ln\n" if (!defined $sentence);
	    print STDERR "sentence=$sentence\n" if ($verbose > 1);

	    my $tagger_out = '';
	    if ($run_tagger) {
		print WRITEME "$sentence\n";
		$tagger_out = <README>;
		chomp($tagger_out);
	    } else {
		$tagger_out = $tagged_file[$i++];
	    }
	    print STDERR "output=$tagger_out\n" if ($verbose > 1);

  	    my @taglist = split(/\s+/, $tagger_out);
  	    foreach $wt (@taglist) { 
  		my ($w, $t) = split(/$tagger_delim/, $wt);
		push(@sent, $w);
  		push(@postags, $t);
  	    }
	    
	    print STDERR "taglist=", join(' ', @postags), "\n" if ($verbose > 1);
	}
	print "$_\n"; 
	next; 
    }
    my $prflag = 1;
    my @line = split;

    ## check if the postag reported by the tagger matches with the
    ## possible postag reported by the XTAG syntax lexicon. look up
    ## %posmapping for the map between the tagger postags and the XTAG
    ## postags. if the mapping reports a match then keep the entry
    ## else suppress it. posmapping can have entries like "V|P" in
    ## which case either an XTAG postag of 'V' or 'P' will be
    ## acceptable.

    my $xtagpos = $line[3];
    my $windex  = $line[0];
    $line[2] = $postags[$windex];
    my $posmap = $posmapping{$line[2]};
    if (!defined $posmap) {
	print STDERR "Mapping not found for: ", $line[2], "\n";
    } else {
	$prflag = 0;
	foreach (split(/\|/, $posmap)) {
	    print STDERR "Checking posmap=<$_> against xtag_postag=<$xtagpos>\n" 
		if ($verbose);
	    if ($xtagpos eq $_) {
		$prflag = 1;
		last;
	    }
	}
    }

    if ($line[1] =~ /$whpat/) { $has_wh = 1; }
    if (($has_wh == 0) && (/$whtreepat/)) { $prflag = 0; }
    foreach $pat (@delete_pats) {
	if (/$pat/) {
	    $prflag = 0;
	}
    }

    my $new_line = join(' ', @line);

    if ($prflag) { 


	## check if the previous word had at least one entry
	## printed. If not, then supply some default entries defined
	## in @default_entries.

	my $prev_index = $line[0] - 1;
	if (($line[0] > 0) and (!defined ($register[$prev_index]))) {
	    print STDERR ("Warning: all entries deleted for entry \"", $sent[$prev_index], 
			  "\", index: $prev_index in sent=\"", join(' ', @sent), "\"\n")
		if ($verbose);
	    foreach $default (@default_entries) {
		printf $default, $prev_index, $sent[$prev_index], $postags[$prev_index];
	    }
	    $register[$prev_index] = 1;
	}

	## register the fact that at least one entry for a particular
	## word has been printed
	$register[$line[0]] = 1;

	if ($prev ne $_) { print "$new_line\n"; }
	$prev = $_;

    } else {

	print STDERR "Warning: pruned entry $new_line\n" if ($verbose); 
    }
}

if ($run_tagger) {
    close(README);
    close(WRITEME);
}

