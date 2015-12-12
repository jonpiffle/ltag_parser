# -*- perl -*-

package main;
use English;
use Carp;

use strict;
no strict 'vars';
use integer;

require 'morph.ph';

local($morph_linenumber) = 0;
@morph_tbl[$MAX_MORPHSIZE] = ();

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
    foreach $file (@files) {
	open(MORPHF, $file) || confess "cannot open $file";
	print STDERR "loading morph file: $file\n";

	my($morph_idx) = 0;

	while (<MORPHF>) {
	    $morph_linenumber++;
	    next if /^;/;	# comments
	    next if /^$/;
	    s/\s*$//o;		# remove trailing spaces

	    my($index, $entries) = split(/\s+/, $_, 2);
	    my(@entries) = split(/\#/, $entries);
	    foreach (@entries) {
		my($entry, $pos, $expanded_fs) = &morph_entry($_);
		print "$index\t$entry\t$pos\t$expanded_fs\n";
	    }
	}
	close(MORPHF);
    }
}

sub morph_entry
{    my($entry) = @ARG;
     my(@entry_split) = split(/\s+/, $entry);
     
     # find the pos label, left of it is the entry
     # and right of it are the features
     my($pos_index) = &find_split(reverse @entry_split);
     my($len) = $#entry_split;
     $pos_index = $len - $pos_index; # unreverse the index
     my($i);

     # collect feature structures
     my(@fs) = ();
     for ($i = $pos_index+1; $i <= $len; $i++) {
	 my($morph_templ) = "@" . $entry_split[$i];
	 push(@fs, $morph_templ);
     }
     my($expanded_fs) = join(" ", @fs);

     # collect entry as a string
     my($entry_val) = "";
     for ($i = 0; $i < $pos_index; $i++) {
	 $entry_val = $entry_val . $entry_split[$i];
     }

     return ($entry_val, $entry_split[$pos_index], $expanded_fs);
}

sub find_split
{    my(@entry_split) = @ARG;
     my($i);
     for ($i = 0; $i <= $#entry_split; $i++) {
	 foreach (@POS_LABELS) {
	     return $i if ($entry_split[$i] eq $_);
	 }
     }
     &confess_morph_error("could not find POS label in entry");
}

1;
