#!@PERLBIN@

my $i = 0;
my $prev = -1;
my $line = 0;
my $multipage = 0;
my $nilstr = "(nil)";
my $insert_or_nodes = 0;
my $insert_boxes = 0;
while (<>)
{
    chomp;
    $line++;
    if (/^begin/) {
	die "forest not terminated at line $line.\n" if ($i == $prev);
	/\"([^\"]*)\"/;
	print STDERR "producing graph for \"$1\"\n";
	print "digraph G$i {\n";
        if ($multipage) {
	   #multiple page output
	   print "   page=\"8.5,11\";\n";
	   print "   ratio=auto;\n";
        } else {
	   #single page output
	   print "   size=\"7.5,10\";\n";
	   print "   ratio=fill;\n";
        }
	print "   rankdir=LR;\n";
	$prev = $i;
	next;
    }
    if (/^end/) {
	print "}\n";
	$i++;
	next;
    }
    if (/^start/) {
        s/\[[^\]]*\]//g;
	/start: (.*)$/;
	print "   start -> N$1;\n";
	next;
    }

    # handle features if they exist
    s/ \#/:\#/go;
    s/ \>/\>/go;

    my ($n, $rest) = split(':', $_, 2);
    $rest =~ s/^\s*//o;
    my ($l, $ns) = split(' ', $rest, 2);

    # remove excess information from nodelabel
    $l =~ s/\>.*$/\>/o;

    print "   N$n [label=\"$l\"];\n";

    $ns =~ s/^\s+//o;
    $ns =~ s/\s+$//o;
    next if ($ns =~ /^$/);

    $ns =~ s/^\[//o;
    $ns =~ s/\]$//o;
    @ns_list = split(/\]\[/, $ns);
    my $orc = 0;
    foreach (@ns_list) { 
  	my ($b, $c) = split(',', $_, 2);
	if ($insert_or_nodes) {
	    print "   N$n -> N$n", "_", "OR$orc;\n";
	    print "   N$n", "_", "OR$orc", " [label=\"$l", "_", "OR$orc\"];\n";
	}
  	if ($b ne $nilstr) {
	    if ($insert_or_nodes) { print "   N$n", "_", "OR$orc -> N$b;\n"; }
	    else { print "   N$n -> N$b;\n"; }
  	}
  	if ($c ne $nilstr) {
	    if ($insert_or_nodes) { print "   N$n", "_", "OR$orc -> N$c;\n"; }
	    else { print "   N$n -> N$c;\n"; }
  	}
        if ($insert_boxes) {
	  if (($b ne $nilstr) || ($c ne $nilstr)) {
	      print "subgraph clusterN$n", "_", "$orc { ";
              print "N$b " if ($b ne $nilstr);
              print "N$c " if ($c ne $nilstr);
	      print "}\n";
	  }
	}
	$orc++;
    }
}
