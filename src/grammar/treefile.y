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
my($yyinputf) = "";			# contains input for yylex
my($yylinenumber) = 0;		# line number in current file
my($has_foot) = 0;
my($verbose) = 0;

%}

%token STRING NUMBER SYMBOL

%start tree_file

%%

tree_file:
	|	tree_file tree
	;

tree:		tree_header tree_nodes
                {
		    if (defined $tree_tbl{$1->[0]}) {
			print STDERR "ERROR: duplicate tree names in grammar (", $1->[0], ")\n";
			exit;
		    }
		    $tree_tbl{$1->[0]} = [ $2, $1->[1] ];
		    push(@{$tree_list}, $1->[0]);
		    if ($verbose >= 2) {
			print STDERR $1->[0], "\n" if ($has_foot);
			$has_foot = 0;
		    }
                }
	;

tree_header:	'(' tree_name attr_list ')'
		{
		    my(@alist) = @{$3};
		    my(%alist) = @alist;
		    my($features) = "";
		    foreach $_ (keys %alist) { 
			if ($_ eq "UNIFICATION-EQUATIONS") {
			    $features = $alist{$_};
			    last;
			}
		    }
		    $$ = [ $2, $features ];
		}
     	;

tree_name:	STRING 
		{ 
		    $$ = $1;
		}
	;

tree_nodes:	node
		{
		    $$ = $1;
		}
	;

node:		'(' node_detail ')'
		{
		  $$ = $2;
		}
	;

node_detail:	'(' node_name attr_list ')' child_list
		{
		    my(@alist) = @{$3};
		    my(%alist) = @alist;
		    my($node_type) = "(substp|footp|headp)";
		    my($constraints) = "constraint-type";
		    my(@info);
		    push(@info, $2);
		    foreach (keys %alist) {
			push(@info, $_) if (/$node_type/i);
			push(@info, $alist{$_}) if (/$constraints/i);
		    }
		    if ($verbose >= 2) {
			if (grep /footp/, @info) { 
			    print STDERR join(":", @info), " "; 
			    $has_foot = 1;
			}
		    }
		    $$ = &Tree::new($5, join(":", @info));
		}
	;

node_name:	'(' '(' STRING '.' STRING ')' ')'
		{ 
		  if ($5 eq "") { $$ = $3; }
		  else { $$ = $3 . "_" . $5; }
	        }
	;

child_list:			
		{
		    $$ = $Tree::NULL_T;
		}
	|	children	
		{ 
		    $$ = $1;
		}
	;

children:	node
		{
		    $$ = $1;
		}
	|	node children
		{
		    &Tree::insert_sibling($1, $2);
		    $$ = $1;
		}
	;

attr_list:		{ $$ = []; }
	|	attrs	{ $$ = $1; }
	;

attrs:		attr		{ $$ = $1; }
	|	attrs attr	{ push( @{$1}, @{$2}); $$ = $1; }
	;

attr:		keyword attr_value	
		{ 
		    $$ = [ $1 , $2 ]; 
		}
	;

attr_value:	list	{ $$ = $1; }
	|	symbol	{ $$ = $1; }
	|	NUMBER	{ $$ = $1; }
	;

list:		'(' list_elts ')'	{ $$ = $2; }
	;

list_elts:	symbol			{ $$ = $1; }
	|	list_elts symbol	{ $$ = $1 . " " . $2; }
	;

symbol:		SYMBOL		{ $$ = $1; }
	|	STRING		{ $$ = $1; }
	|	val_keyword	{ $$ = $1; }
	;

keyword:	':' SYMBOL	{ $$ = $2; }
	|	SYMBOL		{ $$ = $1; }
	;

val_keyword:    ':' SYMBOL	{ $$ = $2; }
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

	if ($char =~ /^[\"]/) { # "
	    # collect a string, recognizes backslashed double quotes inside the string
	    if ($yyinputf =~ s/^(([^\\$char]|\\.)*)$char//o) {
		$yylval = $1;
	    } else {
		&yyerror('unterminated string');
		$yylval = '';
	    }
	    $yylval =~ s//$Tree::ALPHA_S/o; # Xtag convention for Greek letters
	    $yylval =~ s//$Tree::BETA_S/o;
	    $yylval =~ s//$Tree::EPSILON_S/o;
	    &count($yylval);
 	    $STRING;
        } elsif ($char =~ /^\d/) {
	    # collect a number. does not recognize floats.
	    if ($yyinputf =~ s/^(\d+)//o) {
		$yylval = int($char . $1);
	    } else {
		$yylval = int($char);
	    }
	    &count($yylval);
	    $NUMBER;
	} elsif ($char =~ /^\w/) {
	    # collect a symbol
	    if ($yyinputf =~ s/^([\w\?-]+)//o) {
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
