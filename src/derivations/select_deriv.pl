#!@PERLBIN@ -w

# select_deriv
# Author: Kate Forbes <forbesk@unagi.cis.upenn.edu>

my $rcsid = '$Id: select_deriv.pl,v 1.2 2001/01/30 01:11:50 anoop Exp $';
my $verbose = 0;
print STDERR "$rcsid\n" if ($verbose > 1);

# This program is derived from Anoop Sarkar's print_deriv. Takes user
# input lexical items and tree names and retrieves all derivations
# containing those trees.  Program works by recursively traversing the
# forest from the start nodes.  For each of the daughter pairs, if
# either of them points to a node that matches an input lexical item
# but does not match the corresponding input tree name, that branch is
# excised and returns 'true' to its parent. Daughter pairs that return
# true are excised from the parent.  If all daughters of a branch are
# excised the parent branch is excised and returns 'true' upwards.

# IMPORTANT NOTE: if the input sentence contains identical lexical
# items that take different trees, both items and their trees must be
# input at once (e.g. 'if John leaves Mary leaves the office.'  user
# must input 'leaves alphanx0V leaves alphanx0Vnx1' at the same time).

%graph = ();
%linegraph = ();
%start = ();
%mult = ();
$readfile = $ARGV[0];
$writefile = "";
$num = 1;
$inpt = "";
$name = "";
$tree = "";
$sent = "";
$leaf = '__leaf__';

if (defined $ARGV[1]) {

    print "printing print_deriv output in: $ARGV[2]\n"; 
    $store_deriv = $ARGV[2];
}

while ($inpt ne 'quit') { 

    system ("count_derivations $readfile");
    print "Would you like to print_deriv -d? (y or n) ";
    chop($answer = <STDIN>);
    if ($answer eq 'y') {
	if (defined $store_deriv) {
	    system ("print_deriv -d $readfile > $store_deriv");
	}
	else {
	    system ("print_deriv -d $readfile");
	}
    }
    
    print <<EOM;
(type 'quit' to end, 'extract' to extract_first) 
Enter a sequence of lexical items and their correct tree names 
(e.g. however betaARBvx unwilling alphanx0Ax1) 
If two lexical items in input are identical but take different tree names, you must enter both
EOM
    print "(e.g. leaves alphanx0V leaves alphanx0Vnx1): ";

    chop($inpt = <STDIN>);

    if ($inpt eq 'extract') {
	$readfile =~ s/^(.*p1.)(.*)/$2/;
	system ("extract_first $readfile > $readfile.best");
	print "your extracted parse is in: $readfile.best\n";
	$inpt = 'quit';
    }
    if ($inpt ne 'quit') {
	my @in = split(' ', $inpt);

	for ($t=0; ($t*2)<(@in+0); $t++) {
	    for ($u=$t; ($u*2+2)<(@in+0); $u++) {
		if ($in[$t*2] eq $in[$u*2+2]) {
		    $mult{$in[$t*2+1]} = $in[$t*2]; 
		    $mult{$in[$u*2+3]} = $in[$u*2+2];
		}
	    }
	}
	open(B, "$readfile") || die "Cannot open $readfile\n";
	while (<B>) {

	    chop($myline = $_);
	    if ($myline =~ /^begin/) {
		$sent = $myline;
	    }
	    elsif ($myline =~ /^end/) { 

		print "done storing forest\n";
		compute_deriv();
		print "done computing new forest\n";

		$writefile = "p$num.$readfile";
		open (D, ">$writefile") || die "cannot open $writefile";
		print D "$sent\n";

		foreach $dtr (sort keys %start) {
		    print D "start: $dtr\n"; 
		    clean_deriv($dtr);
		}
		print D "end\n";
		close D;
		print "done printing new file: $writefile\n";
		$readfile = $writefile;
		$num++;
		%graph = ();
		%linegraph = ();
		%start = ();
		%mult = ();
	    }
	    else  {
		$save = $myline;
		$myline =~ s/ \#/;\#/go;
		$myline =~ s/ \>/\>/go;

		my ($node_id, $label, $dtrs) = split (' ',$myline);
		$node_id =~ s/:$//o;

		if ($node_id =~ /^start$/) { 
		    $start{$label} = 'found';
		}
		else {
		    $linegraph{$node_id} = $save;
		    for ($t=0; ($t*2)<(@in+0); $t++) {
			$name = $in[$t*2];
			$tree = $in[$t*2+1];
			if ($save =~ /.*\[$name\].*/ && $save !~ /.*$tree\[.*/) { 
			    $found = 'f'; 
			    foreach $k (sort keys %mult) { 
				if ($save =~ /.*$k\[.*/) { 
				    $found = 't'; 
				}
			    }
			    if ($found eq 'f') {
				delete $linegraph{$node_id};
				if (defined $start{$node_id}) {
				    delete $start{$node_id};
				}
			    }
			}
		    }
		    if ((!defined $dtrs) || ($dtrs eq '')) { 
			$dtrs = $leaf; 
		    }
		    if ($dtrs ne $leaf) {
			$dtrs =~ s/\[//go;
			$dtrs =~ s/\]/ /go;
			$dtrs =~ s/\s*$//o;
		    }
		    $graph{$node_id} = $dtrs;	    
		}
	    }
	}
	close B;
    }
}

sub clean_deriv {
    my $id = $_[0]; 
    if ($linegraph{$id} =~ /^.*\#.*$/ || $linegraph{$id} =~ /^.*<>.*$/) {
	print D "$linegraph{$id}\n";
	delete $linegraph{$id};
    }
    else {  
	print D "$linegraph{$id}\n";
	$aline = $linegraph{$id};
	delete $linegraph{$id};
	$aline =~ s/^.*>\[\]\s(\[\d.*)$/$1/;
	$aline =~ s/\[//go;
	$aline =~ s/\]/ /go;
	$aline =~ s/,/ /go;
	$aline =~ s/ \(nil\)//go; 
	$aline =~ s/\s*$//;
	my @dt = split (' ',$aline);
	while ($item = shift @dt) {
	    if (defined $linegraph{$item}) {
		clean_deriv($item);
	    }
	}
    }
}

sub compute_deriv  {
    my $i;
    foreach $s (sort keys %start) {
	my ($n, $truth) = compute_deriv_recursive($s);
	if ($truth eq 't') {           
	    delete $linegraph{$n};
	    delete $start{$n};
	}
    }
}

sub compute_deriv_recursive  {

    my $node_id = $_[0];
    my @dtr_ORlist;

    if ($node_id eq '(nil)') {
	return($node_id,'f');
    }

    if (!defined($graph{$node_id})) {
	die "hey! could not find $node_id in graph table\n";
    }

    if (!defined($linegraph{$node_id})) {
	return($node_id,'t');
    }

    @dtr_ORlist = split(' ', $graph{$node_id});

    if ($#dtr_ORlist < 0) {
	die "hey! found no daughters for node: $node_id\n";
    }

    if ($dtr_ORlist[0] eq $leaf) {
	return($node_id, 'f');
    }
    
    foreach $ANDpair (@dtr_ORlist) {     # traverse daughters:
	my ($xin, $yin) = split(',', $ANDpair);

	my ($xout, $xtruth) = compute_deriv_recursive($xin);
	my ($yout, $ytruth) = compute_deriv_recursive($yin);

	if ($xtruth eq 't' || $ytruth eq 't') {
	    $linegraph{$node_id} =~ s/\[$xout,\(*\w{3,6}\)*\]//g;
	}
    }
    if ($linegraph{$node_id} =~ /.*>\[\]\s*$/) { 		   
	delete $linegraph{$node_id};
	delete $start{$node_id};
	return ($node_id, 't');
    }
    else {
	return ($node_id, 'f');
    }
}




