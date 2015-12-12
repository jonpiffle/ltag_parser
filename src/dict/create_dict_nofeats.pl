#!/usr/bin/perl

# require 'morph.pl';

if ($#ARGV < 0) { die "usage: $0 <syntax-file>\n"; }

if (!(-T $ARGV[0])) { die "$0: $ARGV[0] does not seem to be a valid syntax file\n"; }

use English;
use Carp;

use strict;
no strict 'vars';
use integer;

my $syn_linenumber = 0;
my %inverted_syn_tbl;
my $join_symbol = "_";

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
	open(SYNF, $file) or confess "cannot open $file";
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


	    my(@value) = ();
	    my($value) = "";

	    push(@value, $trees) if ($has_trees);
	    foreach (@value) { s//alpha/go; s//beta/go; }
	    push(@value, $family) if ($has_family);
	    # push(@value, $features) if ($has_features);
	    $value = join(" ", @value);

	    $words =~ s/<<INDEX>>//o;
	    @word_pos_list = split(/<<ENTRY>>/, $words);
	    shift(@word_pos_list); # ignore index

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
	    my($index) = join($join_symbol, @pos) . " " . $value;
	    # print $index, "\n";

	    $inverted_syn_tbl{$index} = [] 
		if (!defined($inverted_syn_tbl{$index}));
	    push(@{$inverted_syn_tbl{$index}}, join($join_symbol, @word));

	}
	close(SYNF);
    }
}

sub print_inverted_syn_tbl
{
    foreach $i (sort keys %inverted_syn_tbl)
    {
	print join(" ", @{$inverted_syn_tbl{$i}}), "\n";
    }
}

sub print_dict
{
    my($dir) = "words.nofeats";
    my($file_templ) = "";
    my($num) = 1;
    my($file_suf) = "";

    umask(0);
    mkdir($dir, 0755) or die "could not create directory $dir\n";
    chdir($dir) or die "could not change directory to $dir\n";
    print STDERR "created directory $dir\n";
    foreach $i (sort keys %inverted_syn_tbl)
    {
	my(@t_arr) = split(/ /, $i);
	my($pos) = shift(@t_arr);
	my(@word_list) = @{$inverted_syn_tbl{$i}};
	if ($#word_list == 0) {
	    print $word_list[0], " : ", $i, "\n\n";
	} else {
	    my($file) = $file_templ . $pos . "." . $num . $file_suf;
	    open(F, ">$file") or die "could not open $file for writing\n";
	    print F join("\n", @word_list), "\n";
	    print $file, " : ", $i, "\n\n";
	    print STDERR "wrote ", $#word_list+1, " words to $file\n";
	    close(F);
	    $num++;
	}
    }

}

slurp_syntax($ARGV[0]);
print_dict();

