#!@PERLBIN@ -w

package main;
use English;
use Carp;

$usage = "usage: $0 <parameter-file>\n";
$PARAMS = shift or die $usage;

# Lexicon definition, contains locations of input files 
# and output files 
if (-f $PARAMS) {
    require $PARAMS;
} else {
    die "could not find parameter file: $PARAMS\n";
}

sub vars_defined 
{   my(@var) = @_;
    foreach (@var) {
	die "$_ not defined\n" if !defined eval "$_";
    }
}

# check if all the necessary variables are defined in $PARAMS
&vars_defined(qw(
		 $LANG          
		 $SYN_FILE      
		 $MORPH_FILE    
		 $COMBINED_FILE 
		 $POS_LABELS    
		 $POS_TBLS      
		 $DEFAULTS      
		 $EPSILONS
		 $TEMPLATES     
		 $NEW_TEMPLATES 
		 ));

# parameters
my $insert_defaults = 1; # zero for no defaults
my $verbose         = 0; # lower the quieter
my $print_all_syn   = 1; # if entry is in syn but not in morph then print anyway

sub file_exists
{
    my ($filename) = @ARG;
    if (-f $filename) { return(1); }
    else { die "check $PARAMS: could not find file: $filename\n"; }
}

# load the anchor labels from POS_LABELS
require $POS_LABELS
    if (file_exists($POS_LABELS));

# load the conversion tables:
# define label_to_standard_pos_conversion
# and morph_pos_to_standard_pos_conversion
# and feature_conditionals
require $POS_TBLS
    if (file_exists($POS_TBLS));

# define the default mapping for words in morph that are not found in
# the syntactic lexicon
require $DEFAULTS
    if (file_exists($DEFAULTS));

my %eps_tbl         = ();
my %syn_tbl         = ();
my %morph_templates = ();
my %syn_templates   = ();
my %new_templates   = ();

my $IS_FAMILY = 1; 
my $IS_TREE   = 2; 

my $syn_linenumber   = 0;
my $morph_linenumber = 0;
#@morph_tbl[$MAX_MORPHSIZE] = ();

sub confess_morph_error
{   my($mesg) = @ARG;
    confess "at line $morph_linenumber: $mesg";
}

sub morph_warning
{   my($mesg) = @ARG;
    print STDERR "at line $morph_linenumber: $mesg";
}

sub slurp_morph
{   my(@files) = @ARG;

    open(CMB, ">$COMBINED_FILE") or die "could not open $COMBINED_FILE\n";
    foreach $file (@files) 
    {
	open(MORPHF, $file) or confess "check $PARAMS: cannot open $file";
	print STDERR "loading morph file: $file\n";
	my(%syn_check);
	my($morph_idx) = 0;

	while (<MORPHF>) 
	{
	    $morph_linenumber++;
	    next if /^;/;	# comments
	    next if /^$/;
	    s/\s*$//o;		# remove trailing spaces

	    my($index, $entries) = split(/\s+/, $_, 2);
	    my(@entries) = split(/\#/, $entries);

	    foreach (@entries) 
	    {
		my($entry, $pos, $features) = morph_entry($_);
		#print "Entry = $entry    POS = $pos\n";

		my($syn_entry) = $syn_tbl{$entry};
		if (!defined $syn_entry) {
		    # insert defaults here
		    insert_defaults(*CMB, $index, $pos, $features);
		    print STDERR 
			"INDEX=$index POS=$pos FEAT=$features not in the syntactic lexicon\n" 
			    if ($verbose > 1);
		} else {
		    if (defined $syn_check{$entry}) { $syn_check{$entry}++; }
		    else { $syn_check{$entry} = 1; }

		    # $morph_tbl[$morph_idx] = [ $entry, $pos, $features ]; 
		    # $morph_idx++;

		    print_syn_entry_on_pos_agreement(*CMB, $syn_entry, $entry, $index, $pos, $features);
		}
	    }
	}

	foreach (keys %syn_tbl) {
	    if (!defined $syn_check{$_}) {
		if (defined $eps_tbl{$_}) { 
		    print_syn_entry(*CMB, $syn_tbl{$_}, $_, $_);
		} else {
                    if ($print_all_syn) {
                        print_syn_entry(*CMB, $syn_tbl{$_}, $_, $_);
                    } else {
		        print STDERR "$_ not in morph\n" 
			    if ($verbose);
                    }
		}
	    }
	}

	close(MORPHF);
    }
    close(CMB);
    print STDERR "wrote combined morph and syn to $COMBINED_FILE\n";
} 

sub print_syn_entry_on_pos_agreement
{
    my($stream, $syn_entry, $morph_entry, $index, $morph_pos, $morph_features) = @ARG;

    if (!defined $morph_features) { $morph_features = ''; }

    my $nomatch = 0;
    foreach $syn (@$syn_entry)
    {
	my($entry_list) = $syn->[0];
	my($pos_list) = $syn->[1];

	my($morph_standard_pos) = convert_morph_pos_to_standard($morph_pos, $morph_features);

	my($syn_standard_pos) = 
	    find_entry_pos($index, $morph_standard_pos, $morph_entry, $entry_list, $pos_list);

	if ($verbose > 1) {
	    print STDERR "entry_list=$entry_list\n";
	    print STDERR "pos_list=$pos_list\n";
	    print STDERR "morph_entry=$morph_entry\n";
	    print STDERR "morph_standard_pos=$morph_standard_pos\n";
	    print STDERR "syn_standard_pos=$syn_standard_pos\n"; 
	}

	if ($syn_standard_pos eq "") { $nomatch = 1; next; } 
	else { $nomatch = 0; }

	if ($syn_standard_pos ne $morph_standard_pos) { 
          print STDERR 
	     "INDEX=$index SYN_POS=$syn_standard_pos ",
	     "MORPH_POS=$morph_standard_pos did not match\n" 
	        if ($verbose);
	  return; 
	}

	# print if pos matches
	my $flag          = $syn->[2];
	my $trees         = $syn->[3];
	my $entry_pos     = make_entry_pos($index, $morph_pos, $entry_list, $pos_list);
	my $features      = $syn->[4];
	my $sep_label     = ($flag == $IS_TREE) ? '<<TREES>>' : '<<FAMILY>>';
	my $feature_label = ($features || $morph_features) ? '<<FEATURES>>' : '';
	my $feat_sep      = ($morph_features && $features) ? ' ' : '';

	if ($morph_features ne '') {

	    my $new_morph_ref = new_morph_templ($syn_standard_pos, $morph_features);
	    foreach $new_morph (@{$new_morph_ref})
	    {
		#print STDERR "features: $features :: $new_morph\n";
		print $stream "<<INDEX>>$index", $entry_pos ,
		   $sep_label , "$trees" , $feature_label , $features, 
		   $feat_sep, $new_morph, "\n";
	    }
	    
	} else {

	    #print STDERR "features: $features :: $new_morph\n";
	    print $stream "<<INDEX>>$index", $entry_pos ,
	       $sep_label , "$trees" , $feature_label , $features, "\n";

	}
    }

    if ($nomatch) {
	# insert defaults here
	insert_defaults($stream, $index, $morph_pos, $morph_features);
	print STDERR 
	    "ENTRY=$index MORPH_POS=$morph_pos FEAT=$morph_features ",
	    "not in the syntactic lexicon\n" 
		if ($verbose > 1);
    }
}

sub convert_morph_pos_to_standard
{
    my($morph_pos, $morph_features) = @ARG;
    my($r);
    foreach $feat (keys %feature_conditionals) {
	if ($morph_features =~ /$feat/) {
	    return($feature_conditionals{$feat});
	}
    }
    if (defined $morph_pos_to_standard_pos_conversion{$morph_pos}) {
	$r = $morph_pos_to_standard_pos_conversion{$morph_pos};
    } else {
	$r = $morph_pos;
    }
    return($r);
}

sub find_entry_pos
{    my($index, $morph_pos, $morph_entry, $entry_list, $pos_list) = @ARG;
     my(@entries) = split(/\s+/, $entry_list);
     my(@pos) = split(/\s+/, $pos_list);
     for ($i = 0; $i <= $#entries; $i++) {
	 my($syn_standard_pos);
	 if (defined $label_to_standard_pos_conversion{$pos[$i]}) {
	     $syn_standard_pos = $label_to_standard_pos_conversion{$pos[$i]};
	 } else {
	     $syn_standard_pos = $pos[$i];
	 }
	 if (($entries[$i] eq $morph_entry) && ($syn_standard_pos eq $morph_pos)) {
	     return($syn_standard_pos);
	 } 
         print STDERR "$entries[$i]==$morph_entry; $syn_standard_pos==$morph_pos did not succeed\n"
             if ($verbose > 1);
     }
    return("");
}

sub print_syn_entry
{
    my($stream, $syn_entry, $entry, $index) = @ARG;

    foreach $syn (@$syn_entry) 
    {
	my($entry_list) = $syn->[0];
	my($pos_list)   = $syn->[1];
	my($flag) = $syn->[2];
	my($trees) = $syn->[3];
	my($entry_pos) = make_entry_pos($index, $entry, $entry_list, $pos_list);
	my($features) = $syn->[4];
	my($sep_label) = ($flag == $IS_TREE) ? "<<TREES>>" : "<<FAMILY>>";
	my($feature_label) = $features ? "<<FEATURES>>" : "";
	print $stream "<<INDEX>>$index", $entry_pos ,
	   $sep_label , "$trees" , $feature_label , "$features\n";
    }
}

sub process_disjuncts
{
    my($morph_feats) = @ARG;

    my @out = ($morph_feats);
    my $i;
    my $m;

    foreach (split(' ', $morph_feats))
    {
	if ($feat_disjuncts{$_}) { 
	    my @disjuncts = split(' ', $feat_disjuncts{$_});
	    my $sz = $#disjuncts+1;
	    my @new = ();
	    while ($m = shift(@out))
	    {
		for ($i=0; $i < $sz; $i++)
		{
		    my $tmp = $m;
		    $tmp =~ s/$_/$disjuncts[$i]/;
		    push(@new, $tmp);
		}
	    }
	    @out = @new;
	} 
    }
    #print STDERR join("\n", @out), "\n";
    return(\@out);
}

# changes a morph feat from G.b:<ref>=<agr num>=plur
# to G.b:<ref>=G.b:<agr num>, G.b:<agr num>=plur
sub replace_mult_eqs_in_morph_feats
{
    my ($fsref) = @ARG;
    my @out = ();
    foreach $fs (@{$fsref})
    {
	my @items = split('=', $fs);
	my $len = $#items;
	if ($len < 2) { push(@out, $fs); next; }
	my $i;
	my ($node, $rest) = split(':', $items[0]);
	my @nfs = ();
	for ($i=1; $i<$len; $i++)
	{
	    push(@nfs, "$items[0]=$node:$items[$i]");
	}
	push(@nfs, "$node:$items[$len-1]=$items[$len]");
	push(@out, join(", ", @nfs));
    }
    return(\@out);
}

sub add_to_new_templates
{
    my($postag, $morph_templ, $orig) = @ARG;

    my $new_templ = '#' . $postag . '.m_' . $_;
    if (!defined $new_templates{$new_templ}) {
	my @morph_fs = split("\201", $morph_templates{$morph_templ});

	foreach (@morph_fs) {
	    s/^/$postag.b:/;
	}
	my $repl_morph = replace_mult_eqs_in_morph_feats(\@morph_fs);
	my $new_morph = join("\201", @{$repl_morph});
	$new_templates{$new_templ} = $new_morph;
    }
    return($new_templ);
}

sub new_morph_templ
{
    my($postag, $morph_feats) = @ARG;

    my $new_morph_ref = process_disjuncts($morph_feats);
    my @all = ();
    foreach $morph_features (@{$new_morph_ref})
    {
	my @morph_list = split(' ', $morph_features);
	my @out = ();
	foreach (@morph_list)
	{
	    my $morph_templ = '@' . $_;
	    $morph_templ =~ s/(.*)/\U$1\E/;
	    if (defined $morph_templates{$morph_templ}) {
		my $new_templ = add_to_new_templates($postag, $morph_templ, $_);
		push(@out, $new_templ);
	    } else {
		print STDERR "Warning: $morph_templ not found in morph_templates\n";
		#push(@out, $morph_templ);
	    }
	}
	push(@all, join(" ", @out));
    }
    return(\@all);
}

sub check_conflicts
{
    my ($orig_feats, $default_feats) = @ARG;

    if ($default_feats eq '') { return(''); }
    if ($orig_feats eq '') { return($default_feats); }

    my @defaults = split(' ', $default_feats);
    my @new_defaults = ();

    foreach (@defaults)
    {
	my $flag = 1;
	if (defined $feat_conflicts{$_}) {

	    foreach (split(' ', $feat_conflicts{$_})) {
		if (member($_, $orig_feats)) {
		    $flag = 0;
		    last;
		} 
	    }

	}
	push(@new_defaults, $_) if ($flag);
    }
#      print STDERR 
#  	"check_conflicts:: orig_feats: $orig_feats :: new_feats: ", 
#  	join(" ", @new_defaults), "\n";
    return(join(' ', @new_defaults));
}

sub member
{
    my ($x,$lref) = @_;
    @l = split(' ', $lref);
    foreach (@l) {
        if ($_ eq $x) { return(1); }
    }
    return(0);
}

sub morph_entry
{    my($entry) = @ARG;
     my @entry_split = split(/\s+/, $entry);
     
     # find the pos label, left of it is the entry
     # and right of it are the features
     my $pos_index = find_split(reverse @entry_split);
     my $len = $#entry_split;
     $pos_index = $len - $pos_index; # unreverse the index
     my($i);

     # collect feature structures
     my(@fs) = ();
     for ($i = $pos_index+1; $i <= $len; $i++) {
	 push(@fs, $entry_split[$i]);
     }
     my($features) = join(" ", @fs);

     # collect entry as a string
     my($entry_val) = "";
     for ($i = 0; $i < $pos_index; $i++) {
	 $entry_val = $entry_val . $entry_split[$i];
     }

     return ($entry_val, $entry_split[$pos_index], $features);
}

sub find_split
{    my(@entry_split) = @ARG;
     my($i);
     if ($#POS_LABELS < 0) { 
	 print STDERR "Warning: no part-of-speech labels defined\n"; 
	 exit(-1);
     }
     for ($i = 0; $i <= $#entry_split; $i++) {
	 foreach (@POS_LABELS) {
	     return $i if ($entry_split[$i] eq $_);
	 }
     }
     confess_morph_error("could not find POS label in entry");
}

sub make_entry_pos
{   my($index, $morph_index, $entry_list, $pos_list) = @ARG;
     my(@entries) = split(/\s+/, $entry_list);
     my(@pos) = split(/\s+/, $pos_list);
     my(@s);
     for ($i = 0; $i <= $#entries; $i++) {
	 push(@s, "<<ENTRY>>");
	 if ($pos[$i] eq $morph_index) {
	     push(@s, $index);
	 } else {
	     push(@s, $entries[$i]);
	 }
	 if ($pos[$i]) {
	     push(@s, "<<POS>>");
	     push(@s, $pos[$i]);
	 } else {
	     confess "could not assign POS to $index";
	 }
     }
     return(join('', @s));
}

sub confess_syn_error
{   my($mesg) = @ARG;
    confess "at line $syn_linenumber: $mesg";
}

sub syn_warning
{   my($mesg) = @ARG;
    print STDERR "at line $syn_linenumber: $mesg";
}

sub slurp_syntax
{   my(@files) = @ARG;

    foreach $file (@files) {
	open(SYNF, $file) or confess "check $PARAMS: cannot open $file";
	print STDERR "loading syntax file: $file\n";

	while (<SYNF>) {
	    $syn_linenumber++;
	    next if /^$/;
	    s/\s*$//o;		# remove trailing spaces

	    my($entry, $features, $trees, $family);
	    my($has_features, $has_trees, $has_family);

	    if (/<<FEATURES>>/) {
		$has_features = 1;
		($entry, $features) = split(/<<FEATURES>>/, $_, 2);
	    } else {
		$has_features = 0;
		$entry = $_;
		$features = "";
	    }

	    if (/<<TREES>>/) {
		$has_trees = 1;
		($words, $trees) = split(/<<TREES>>/, $entry, 2);
	    } else {
		$has_trees = 0;
		$trees = "";
		if (/<<FAMILY>>/) {
		    $has_family = 1;
		    ($words, $family) = split(/<<FAMILY>>/, $entry, 2);
		} else {
		    $has_family = 0;
		    $words = $entry;
		    $family = "";
		}
	    }

	    if (!$has_trees && !$has_family) {
		syn_warning("no tree or family in entry: " . "$_" . "\n");
		next;
	    }

	    $words =~ s/<<INDEX>>//o;
	    @word_pos_list = split(/<<ENTRY>>/, $words);

	    my $index = shift(@word_pos_list); 

	    my(@word, @pos) = ();
	    my($word_pos, $t_word, $t_pos);
	    while (@word_pos_list)
	    {
		$word_pos = shift(@word_pos_list);
		($t_word, $t_pos) = split(/<<POS>>/, $word_pos);
		confess_syn_error("no word in entry\n") if ($t_word eq "");
		confess_syn_error("no POS in entry\n") if ($t_pos eq "");
		push(@word, $t_word);
		push(@pos, $t_pos);
	    }

	    my($new_entry) = [
			      join(" ", @word),                   # ENTRY = 0
			      join(" ", @pos),                    # POS   = 1
			      $has_trees ? $IS_TREE : $IS_FAMILY, # FLAG  = 2
			      $has_trees ? $trees : $family,      # TREES = 3
			      $features,                          # FS    = 4
			      ];

	    my($old_entry) = $syn_tbl{$index};
	    if (defined $old_entry) {
		push(@$old_entry, $new_entry);
	    } else {
		$syn_tbl{$index} = [ $new_entry ];
	    }

	}
	close(SYNF);
    }
}

sub slurp_epsilons
{
    my($file) = @ARG;
    open(EPSF, $file) or confess "check $PARAMS: cannot open $file";
    print STDERR "reading epsilons from $file\n";
    while(<EPSF>) 
    {
	my ($eps, $pos) = split(/\s+/, $_, 2);
	$eps_tbl{$eps} = $pos;
    }
    close(EPSF);
}

sub slurp_templates
{
    my($file) = @ARG;
    my $line = 1;
    my $key;

    open(TEMPL, $file) or confess "check $PARAMS: cannot open $file";
    print STDERR "reading feature structures from $file\n";
    while (<TEMPL>)
    {
	chomp;
	next if (/^;/);
	next if (/^\s*$/);

	# a valid line must have ! as the last char
	s/!.*$/!/;
	if ($_ !~ /!$/) {
	    print STDERR
		"Error: end of line not terminated by ! at line number: $line\n",
		"line: $_\n";
	    next;
	}

	# clean up the line a bit
	s/\s*!$//;
	s/\s*=\s*/=/g;

	# morph features
	if (/^@/) {
	    my ($m, $v) = split(' ', $_, 2);
	    $m =~ s/(.*)/\U$1\E/;
	    my @fslist = split(',', $v);
	    my @out = ();
	    foreach (@fslist)
	    {
		s/^\s*//;
		s/\s*$//;
		$key = $_;
		$key =~ s/(.*)/\U$1\E/;
		if ($morph_templates{$key}) {
		    push(@out, $morph_templates{$key});
		} elsif (/=/) {
		    my ($l, $r) = split('=', $_, 2);
		    $l =~ s/^\s*//; $l =~ s/\s*$//;
		    $r =~ s/^\s*//; $r =~ s/\s*$//;
		    my $key;
		    $key = $l;
		    $key =~ s/(.*)/\U$1\E/;
		    if ($morph_templates{$key}) { 
			$l = $morph_templates{$key}; 
		    }
		    $key = $r;
		    $key =~ s/(.*)/\U$1\E/;
		    if ($morph_templates{$key}) {
			$r = $morph_templates{$key};
		    }
		    push(@out, "$l=$r");
		} else {
		    push(@out, $_);
		}
	    }
	    if (defined $morph_templates{$m}) {
		print STDERR "Warning: redefining previous definition of $m\n";
	    }
	    $morph_templates{$m} = join("\201", @out);
	    next;
	}

	# syn features
	if (/^#/) {
	    my ($m, $v) = split(' ', $_, 2);
	    if (defined $syn_templates{$m}) {
		print STDERR "Warning: redefining previous definition of $m\n";
	    }
	    $syn_templates{$m} = $v;
	    next;
	}
	
	print STDERR "strange line: $_ at line number: $line\n";
	$line++;
    }
    close(TEMPL);
}

sub print_templates
{
    my $key;
    foreach $key (keys %morph_templates)
    {
	print STDERR "morph:", $key, " \t ", join(" ", split("\201", $morph_templates{$key})), "!\n";
    }
    foreach $key (keys %syn_templates)
    {
	print STDERR "syn:", $key, " \t ", $syn_templates{$key}, "!\n";
    }
    foreach $key (keys %new_templates)
    {
	print STDERR "new:", $key, " \t ", join(", ", split("\201", $new_templates{$key})), "!\n";
    }
}

sub write_new_templates
{
    my $key;
    open(F, ">$NEW_TEMPLATES") or die "cannot open $NEW_TEMPLATES\n";
    foreach $key (keys %new_templates)
    {
	print F $key, " \t ", join(", ", split("\201", $new_templates{$key})), "!\n";
    }
    foreach $key (keys %syn_templates)
    {
	print F $key, " \t ", $syn_templates{$key}, "!\n";
    }
    close(F);
}

sub insert_defaults
{
    my ($stream, $index, $morph_pos, $morph_features) = @ARG;
    if ($insert_defaults == 0) { return; }

    if (!defined($morph_features)) { $morph_features = ''; }

    my $features = '';
    if (defined $feat_defaults{$morph_pos}) {
	$features = $morph_features . 
	    (($morph_features) ? ' ' : '') . 
		$feat_defaults{$morph_pos};
    } else {
	$features = $morph_features;
    }

    my($standard_pos) = convert_morph_pos_to_standard($morph_pos, $morph_features);

    if (defined $family_defaults{$morph_pos}) {
	my $families = $family_defaults{$morph_pos};
	my $syn_entry = [[ $index , $standard_pos , $IS_FAMILY , $families , "" ]];
	print_syn_entry_on_pos_agreement($stream, $syn_entry, $index, $index, $morph_pos, $features);
    }
    if (defined $tree_defaults{$morph_pos}) {
	my $trees = $tree_defaults{$morph_pos};
	my $syn_entry = [[ $index , $standard_pos , $IS_TREE , $trees , "" ]];
	print_syn_entry_on_pos_agreement($stream, $syn_entry, $index, $index, $morph_pos, $features);
    }
}

$dir = $COMBINED_FILE;
$dir =~ s|/[^/]*$||;
if (!(-d $dir)) {
    die "Error: cannot proceed: could not find directory $dir\n";
}

slurp_epsilons($EPSILONS)
    if (file_exists($EPSILONS));
slurp_templates($TEMPLATES)
    if (file_exists($TEMPLATES));
slurp_syntax($SYN_FILE)
    if (file_exists($SYN_FILE));
slurp_morph($MORPH_FILE)
    if (file_exists($MORPH_FILE));
write_new_templates();

