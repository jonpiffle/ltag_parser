# -*- perl -*-

package Tree;
# require Exporter;
# @ISA = qw(Exporter);
# @EXPORT = qw(new insert_sibling search rdump);
# @EXPORT_OK = qw(@tnode_tbl);

use English;
use Carp;

use strict;
no strict 'vars';
use integer;

#--------------------------------------------------
# EXPORTED FUNCTIONS
#--------------------------------------------------

BEGIN 
{ 
    require 'tree.ph';
    $Tree::CURINDEX = $Tree::START_T;
}

sub new 
{   my($leftchild, $info) = @ARG;
    if (@ARG != 2) {
	confess  "usage: new leftchild info";
    }

    # create a new entry in the tree node table
    $tnode_tbl[$CURINDEX] = {
	 "LEFTCHILD" => $leftchild,
	 "RIGHT" => $CURINDEX,
	 "FLAG" => $TRUE,
	 "INFO" => $info,
	 };

    # set RIGHT ptr of rightmost child to point to its parent
    # the RIGHT ptr of the rightmost child starts off pointing
    # to itself
    if ($leftchild != $NULL_T) {
	my($i) = $leftchild;
	while ($tnode_tbl[$i]->{"FLAG"} != $TRUE) {
	    $i = $tnode_tbl[$i]->{"RIGHT"};
	}
	$tnode_tbl[$i]->{"RIGHT"} = $CURINDEX;
    }
	
    # set location for the next node to be inserted
    $CURINDEX++;
}

# insert_sibling(x,y)
# makes y the right sibling of x
sub insert_sibling 
{   my($index, $rightsib) = @ARG;
    if (@ARG != 2) {
	confess  "usage: insert_sibling x, y\nmakes y the right sibling of x";
    }

    # set RIGHT ptr of a node to point its right sibling
    $tnode_tbl[$index]->{"RIGHT"} = $rightsib;

    # FLAG indicates if RIGHT points to parent
    $tnode_tbl[$index]->{"FLAG"} = $FALSE;
}

sub search 
{   my ($index) = @ARG;
    if (@ARG != 1) { 
	confess "usage: search index";
    } 

    $tnode_tbl[$index];
}

sub rdump 
{   my ($node, $stream) = @ARG;
    if (@ARG != 2)  { 
	confess "usage: rdump rootindex stream";
    }

    return if ($node == $NULL_T);
    my($ptr) = $tnode_tbl[$node];
    print $stream "(" if ($ptr->{"LEFTCHILD"} != $NULL_T);
    print $stream $ptr->{"INFO"}, " ";
    &rdump($ptr->{"LEFTCHILD"}, $stream);
    print $stream ") " if ($ptr->{"LEFTCHILD"} != $NULL_T);
    if ($ptr->{"FLAG"} == $FALSE) {
	&rdump($ptr->{"RIGHT"}, $stream);
    }
}

END {
    print STDERR "Total elements found in tnode_tbl = ", $CURINDEX, "\n";
}

1;

