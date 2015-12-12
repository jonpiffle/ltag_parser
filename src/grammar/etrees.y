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


%}

%token SYMBOL

%start tree_file

%%

tree_file:
	|	dump_tree tree_file
        ;

dump_tree:      header tree
                {
		    print STDERR "saving tree: ", $1, "\n";
		    $tree_tbl{$1} = [ $2, "" ];
		    $i++;
                }
	;

header:         SYMBOL '=' SYMBOL
                {
		    if ($1 =~ "tree_number") { $$ = $3; }
		    else { print STDERR "Warning: unknown keyword: $1\n"; $$ = "ERROR_$i"; }
                }
        ;

tree:           '(' node children ')'
                {
                    $$ = &Tree::new($3, $2);
                }
        |       '(' node ')'
                {
                    $$ = &Tree::new($Tree::NULL_T, $2);
                }
        ;

node:           SYMBOL
                {
                    #print STDERR "$i:", $1, "\n";
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

        if ($char =~ /^[^ \t\n()=]/) {
	    # collect a symbol
	    if ($yyinputf =~ s/^([^ \t\n()=]+)//o) {
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
