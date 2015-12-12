# -*- perl -*-

%{

package main;

use Tree;
require 'tree.ph' 
    or die "Installation botched. Could not find tree.ph\n";

use strict;
no strict 'vars';
use integer;

#--------------------------------------------------
# GLOBALS
#--------------------------------------------------
my($ARGV) = "";			# contains current filename
my($yyinputf) = "";		# contains input for yylex
my($yylinenumber) = 0;		# line number in current file
my($has_foot) = 0;
my($verbose) = 0;
my($i) = 0;
my @treenames = ();
my @treefeats = ();
my($name) = "";
my($feat) = "";

%}

%token NUMBER SYMBOL

%start tree_file

%%

tree_file:
	|	dump_tree tree_file
        ;

dump_tree:      tree
                {
		    if (!defined $treenames[$i]) {
			print STDERR "could not find treename for tree: $i\n";
			$name = $i;
		    } else {
			$name = $treenames[$i];
		    }
		    if (!defined $treefeats[$i]) {
			$feat = "";
		    } else {
			$feat = $treefeats[$i];
		    }
		    $tree_tbl{$name} = [ $1, $feat ];
		    $i++;
                }
	;

tree:           '(' node children ')'
                {
                    $$ = &Tree::new($3, $2);
                }
        |       node
                {
                    $$ = &Tree::new($Tree::NULL_T, $1);
                }
        ;

node:           SYMBOL
                {
                    $$ = $1;
                }
        ;

children:       tree
                {
		    $$ = $1;
                }
        |       tree children
                {
		    &Tree::insert_sibling($1, $2);
		    $$ = $1;
                }
        ;

%%

# print an error message
sub yyerror 
{
    print STDERR "\"$ARGV\", " if $ARGV ne '-';
    print STDERR "line $yylinenumber: ", @_, "\n";
}

# count number of newlines 
sub count
{ my($text) = @_;
  my(@t) = split(/\n/,$text);
  $yylinenumber += scalar @t;
}

sub yylex
{
  lexloop:
    {
	$yyinputf =~ s/^([\s\n]*)//o;
	&count($1) if ($1 ne "");
	if ($yyinputf eq "") { close (YYLEX_FILE); return 0; }
	$yyinputf =~ s/^(.)//o; # lookahead character
	my($char) = $1;

        if ($char =~ /^\d/) {
	    # collect a number. does not recognize floats.
	    if ($yyinputf =~ s/^(\d+)//o) {
		$yylval = int($char . $1);
	    } else {
		$yylval = int($char);
	    }
	    &count($yylval);
	    $NUMBER;
	} elsif ($char =~ /^[^ \t\n()]/) {
	    # collect a symbol
	    if ($yyinputf =~ s/^([^ \t\n()]+)//o) {
		$yylval = $char . $1;
	    } else {
		$yylval = $char;
	    }
	    &count($yylval);
	    $SYMBOL;
	} else {
	    $yylval = $char;
	    ord($char);
	}
    }
}

sub yylex_file
{   my($YYLEX_FILE) = @_;
    if (@_ < 1) { 
	print STDERR "usage: yylex_file filename\n";
	return;
    }

    ## we assume that yylex_file will be called only once when using
    ## skel_trees as the input format. If this assumption fails to
    ## hold in the future, the following code that reads the names and
    ## feature files will have to be moved elsewhere as an init step
    ## before yylex_file is first called

    my($treenames, $treefeats) = ("treenames.dat", "treefeats.dat");

    if (-f $treenames) {
	open(F, $treenames) or die "could not open $treenames\n";
    } else {
	die "could not find required file: $treenames\n";
    }

    my $i;

    $i = 0;
    while (<F>) {
	chomp;
	$treenames[$i++] = $_;
    }
    close(F);
    print STDERR "read $i treenames from $treenames\n";

    if (-f $treefeats) {
	open(F, $treefeats) or die "could not open $treefeats\n";
	$i = 0;
	while (<F>) {
	    chomp;
	    $treefeats[$i++] = $_;
	}
	close(F);
	print STDERR "read $i feature structures from $treefeats\n";
    } 

    my($inputsep_stored) = $/;
    $ARGV = $YYLEX_FILE;
    if (! open(YYLEX_FILE, $YYLEX_FILE)) {
	print STDERR "$YYLEX_FILE: cannot open file: $!\n";
	$yyinputf = "";
    } else {
	print STDERR "loading $YYLEX_FILE\n" if $YYLEX_FILE ne '-';
	undef $/;		# slurp in the whole file
	$yyinputf = <YYLEX_FILE>;	# yyinputf is global and used by yylex
    }
    $yylinenumber = 0;	# yylinenumber is global and used by count
    $/ = $inputsep_stored;
}

1;
