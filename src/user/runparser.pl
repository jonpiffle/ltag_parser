#!@PERLBIN@

my $verbose = 0;
my $lemhome='@LEMHOME@';
my $deferror = <<EOM;
LEMHOME not defined
Run 'source setup_paths.sh/csh'

EOM

my $default_grammar_file = "$lemhome/data/english/english.grammar";
my $default_prefs_file = "$lemhome/lib/xtag.prefs";
my $default_parser_bin = "$lemhome/bin/parser.bin";
my $default_nbestparser_bin = "$lemhome/bin/nbest_parser.bin";
my $default_synget_bin = "$lemhome/bin/syn_get.bin";
my $default_deletefilter_bin = "$lemhome/bin/filter";
my $default_checker = "$lemhome/bin/checker";
my $default_lexfeat_file = "$lemhome/data/english/feat_templates.dat";
my $default_gramfeat_file = "$lemhome/data/english/xtag.gram.featdb";
my $default_feat_postprocessor = "$lemhome/bin/slurp_feats.pl";
my $default_server_prefs_file = "$lemhome/lib/server.prefs";

$0 =~ m/.*\/(.*)$/;
my $basename = $1;
my $usage_string = <<EOU;

$basename -- command line interface to the parser

usage: $basename [options] filename ...

options are [-h] [-bg <port>] [+/-l/p/c/u/d/n] [-g/o/r/s/df/fb/fl/fg/fp <filename>] 

  -h          print help
  -bg         run as background server at port number: <port>
              changes options file to $default_server_prefs_file
              use the -o option after this one to change to a different prefs file
              (does not accept filenames in this mode: use parser client)
  +/-l        lowercase first letter of first word in sentence
              default: +
  +/-p        remove sentence final punctuation
              default: +
  +/-n        run parser with input as list of lexicalized trees and with tree filtering
              default: -
  +/-d        run syn get binary with tree deletion filter (used with +n)
              default: +
  -g          use grammar file: <filename>
              default value: $default_grammar_file
  -o          use option/preference file: <filename>
              default value: $default_prefs_file
  -r          run parser binary: <filename>
              default: $default_parser_bin
  -nb         run nbest parser binary: <filename>
              default: $default_nbestparser_bin
  -s          syn get binary (for nbest parser mode): <filename>
              default: $default_synget_bin
  -df         tree deletion filter binary: <filename>
              default: $default_deletefilter_bin
  -pp         pre-processed file passed to deletion filter: <filename>
              default: (none)
  +/-c        unify feature structures in output derivation forest 
              default: -
  +/-u        run only as unifier on an input derivation forest
              does *not* run the parser in this mode
              default: -
  -fb         feature unifier binary: <filename>
              default: $default_checker
  -fl         lexical features filename: <filename>
              default: $default_lexfeat_file
  -fg         grammar features filename: <filename>
              default: $default_gramfeat_file
  -fp         feature postprocessor binary: <filename>
              default: $default_feat_postprocessor
  filename    one sentence per line

Examples:

parsing with tree filtering and without feature unification:
$basename +n filename > outfile

parsing with feature unification:
$basename +c filename > outfile

run feature unification on a forest:
$basename +u outfile > unified_outfile

parsing using trees lexicalized by empty elements:
$basename -r eps_parser.bin filename > outfile

EOU

# default values for options
my $bgserver_opt = 0;
my $bgserver_port = 1111;
my $loweropt = 1;
my $punctopt = 1;
my $checkfeatopt = 0;
my $unifieropt = 0;
my $nbestparseropt = 0;
my $deletefilteropt = 1;
my $grammarfile = $default_grammar_file;
my $prefsfile = $default_prefs_file;
my $parserbin = $default_parser_bin;
my $nbestparserbin = $default_nbestparser_bin;
my $syngetbin = $default_synget_bin;
my $deletefilterbin = $default_deletefilter_bin;
my $deletefilterfile = '';
my $checker = $default_checker;
my $lexfeat_file = $default_lexfeat_file;
my $gramfeat_file = $default_gramfeat_file;
my $feat_postprocessor = $default_feat_postprocessor;

if (!defined $lemhome) {
    print STDERR $deferror;
    exit(0);
} else {
    print STDERR "using parser installed in: $lemhome\n";
}

my @ifiles = ();
my $sz = $#ARGV+1;
my $i;
for ($i = 0; $i < $sz; $i++)
{
    my $file = "";
    my $arg = $ARGV[$i];
    if ($arg =~ /^[-+]/) {
	if ($arg =~ /^-h/) {
	    print $usage_string;
	    exit(0);
	} elsif ($arg =~ /^-bg/) {
	    die $usage_string if ($i+1 >= $sz);
	    my $port = $ARGV[$i+1];
	    if ($port =~ /\d+/) {
		$bgserver_opt  = 1;
		$bgserver_port = $port;
		$prefsfile = $default_server_prefs_file;
		$i++;
	    } else {
		print STDERR "bad port number: $port\n";
		exit(0);
	    }
	} elsif ($arg =~ /^\+l$/) {
	    $loweropt = 1;
	} elsif ($arg =~ /^-l$/) {
	    $loweropt = 0;
	} elsif ($arg =~ /^\+p$/) {
	    $punctopt = 1;
	} elsif ($arg =~ /^-p$/) {
	    $punctopt = 0;
	} elsif ($arg =~ /^\+n$/) {
	    $nbestparseropt = 1;
	} elsif ($arg =~ /^-n$/) {
	    $nbestparseropt = 0;
	} elsif ($arg =~ /^\+d$/) {
	    $deletefilteropt = 1;
	} elsif ($arg =~ /^-d$/) {
	    $deletefilteropt = 0;
	} elsif ($arg =~ /^\+c/) {
	    $checkfeatopt = 1;
	} elsif ($arg =~ /^-c/) {
	    $checkfeatopt = 0;
	} elsif ($arg =~ /^\+u/) {
	    $unifieropt = 1;
	} elsif ($arg =~ /^-u/) {
	    $unifieropt = 0;
	} elsif ($arg =~ /^-g/) {
	    die $usage_string if ($i+1 >= $sz);
	    $file = $ARGV[$i+1];
	    if (-f $file) {
		$grammarfile = glob($file);
		print STDERR "using grammar: $file\n";
		$i++;
	    } else {
		print STDERR "could not find: $file\n";
		print STDERR $usage_string;
		exit(0);
	    }
	} elsif ($arg =~ /^-o/) {
	    die $usage_string if ($i+1 >= $sz);
	    $file = $ARGV[$i+1];
	    if (-f $file) {
		$prefsfile = glob($file);
		print STDERR "using prefs: $file\n";
		$i++;
	    } else {
		print STDERR "could not find: $file\n";
		print STDERR $usage_string;
		exit(0);
	    }
	} elsif ($arg =~ /^-r/) {
	    die $usage_string if ($i+1 >= $sz);
	    $file = $ARGV[$i+1];
	    if (-x $file) {
		$parserbin = glob($file);
		print STDERR "using parser binary: $file\n";
		$i++;
	    } else {
		print STDERR "could not find/execute: $file\n";
		print STDERR $usage_string;
		exit(0);
	    }
	} elsif ($arg =~ /^-nb/) {
	    die $usage_string if ($i+1 >= $sz);
	    $file = $ARGV[$i+1];
	    if (-x $file) {
		$nbestparserbin = glob($file);
		print STDERR "using parser binary: $file\n";
		$i++;
	    } else {
		print STDERR "could not find/execute: $file\n";
		print STDERR $usage_string;
		exit(0);
	    }
	} elsif ($arg =~ /^-s/) {
	    die $usage_string if ($i+1 >= $sz);
	    $file = $ARGV[$i+1];
	    if (-x $file) {
		$syngetbin = glob($file);
		print STDERR "using syn get binary: $file\n";
		$i++;
	    } else {
		print STDERR "could not find/execute: $file\n";
		print STDERR $usage_string;
		exit(0);
	    }
	} elsif ($arg =~ /^-df/) {
	    die $usage_string if ($i+1 >= $sz);
	    $file = $ARGV[$i+1];
	    if (-x $file) {
		$deletefilterbin = glob($file);
		print STDERR "using delete filter binary: $file\n";
		$i++;
	    } else {
		print STDERR "could not find/execute: $file\n";
		print STDERR $usage_string;
		exit(0);
	    }
	} elsif ($arg =~ /^-pp/) {
	    die $usage_string if ($i+1 >= $sz);
	    $file = $ARGV[$i+1];
	    if (-f $file) {
		$deletefilterfile = glob($file);
		print STDERR "passing $file to delete filter binary\n";
		$i++;
	    } else {
		print STDERR "could not find: $file\n";
		print STDERR $usage_string;
		exit(0);
	    }
	} elsif ($arg =~ /^-fb/) {
	    die $usage_string if ($i+1 >= $sz);
	    $file = $ARGV[$i+1];
	    if (-x $file) {
		$checker = glob($file);
		print STDERR "using feature unifier binary: $file\n";
		$i++;
	    } else {
		print STDERR "could not find/execute: $file\n";
		print STDERR $usage_string;
		exit(0);
	    }
	} elsif ($arg =~ /^-fl/) {
	    die $usage_string if ($i+1 >= $sz);
	    $file = $ARGV[$i+1];
	    if (-f $file) {
		$lexfeat_file = glob($file);
		print STDERR "using lexical features: $file\n";
		$i++;
	    } else {
		print STDERR "could not find: $file\n";
		print STDERR $usage_string;
		exit(0);
	    }
	} elsif ($arg =~ /^-fg/) {
	    die $usage_string if ($i+1 >= $sz);
	    $file = $ARGV[$i+1];
	    if (-f $file) {
		$gramfeat_file = glob($file);
		print STDERR "using grammar features: $file\n";
		$i++;
	    } else {
		print STDERR "could not find: $file\n";
		print STDERR $usage_string;
		exit(0);
	    }
	} elsif ($arg =~ /^-fp/) {
	    die $usage_string if ($i+1 >= $sz);
	    $file = $ARGV[$i+1];
	    if (-x $file) {
		$feat_postprocessor = glob($file);
		print STDERR "using feature postprocessor: $file\n";
		$i++;
	    } else {
		print STDERR "could not find/execute: $file\n";
		print STDERR $usage_string;
		exit(0);
	    }
	} else {
	    print STDERR "unknown option: $arg\n";
	    print STDERR $usage_string;
	    exit(0);
	}
    } else {
	push(@ifiles, $arg);
    }
}

my @files = ();
foreach $filename (@ifiles) {
    if (!(-f $filename)) { print STDERR "could not find file $filename\n"; }
    else { push(@files, $filename); }
}

if ($#files == -1) {
    push(@files, "-");
}

my $syngetprogram = "";

if ($deletefilteropt) {
    $syngetprogram .= "| $syngetbin $grammarfile $prefsfile | $deletefilterbin $deletefilterfile ";
}

my $runprogram = "";
if ($nbestparseropt) {
    $runprogram = $syngetprogram . "| $nbestparserbin $grammarfile $prefsfile";
} else  {
    $runprogram = "| $parserbin $grammarfile $prefsfile";
}

if ($checkfeatopt) {
    $runprogram .= " | $checker $lexfeat_file $gramfeat_file" .
	           " | $feat_postprocessor";
} 

if ($unifieropt) {
    $runprogram = "| $checker $lexfeat_file $gramfeat_file" .
                  " | $feat_postprocessor";
}

print STDERR "Executing: $runprogram\n" if ($verbose);

if ($bgserver_opt) {

    use IO::Socket;
    use IPC::Open2;

    my $server_port = $bgserver_port;
    my $server = IO::Socket::INET->new(LocalPort => $server_port,
				       Type      => SOCK_STREAM,
				       Reuse     => 1,
				       Listen    => 10 ) # or SOMAXCONN
	or die "cannot be a tcp server on port $server_port : $@\n";

    $runprogram =~ s/^\| //;

    while ($client = $server->accept()) {

	open2(*README, *WRITEME, $runprogram) or die "could not open2 $runprogram\n";
	die "cannot fork: $!\n" unless defined ($kidpid = fork());
	if ($kidpid) {

	    while (defined ($output = <README>)) {
		print $client $output;
	    }
	    kill("TERM" => $kidpid);

	} else {

	    while (defined ($input = <$client>)) {
		print WRITEME $input;
	    }
	}
	close($client);
	close(README);
	close(WRITEME);
    }

} else {

    open(I, $runprogram) or die "could not execute $runprogram\n";
    foreach $filename (@files) {
	if ($filename eq "-") {
	    *F = STDIN;
	} else {
	    open(F, $filename) or die "cannot open $filename\n";
	}
	while (<F>)
	{
	    chomp;
	    next if /^\s*$/;
	    s/^(.)/\L\1/ if ($loweropt);
	    s/\s*[\.\?\!]$// if ($punctopt);
	    print I "$_\n";
	}
	close(F);
    }
    close(I);

}
