
/* gram_treemap - map functions and collect results over trees */

/* Copyright (c) 2000 Anoop Sarkar
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either version 2 of 
 * the License, or (at your option) any later version. 
 * 
 * The file COPYING in the current directory has a copy of the GPL. 
 *
 * This code comes WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
**/

static char rcsid[] = "$Id: gram_treemap.c,v 1.1 2000/06/06 16:06:37 anoop Exp $";

#include "err.h"
#include "gram.h"

void *
gram_tree_map (const grammar_t *gram, 
	       void *(*new) (const grammar_t *g, int treenode), 
	       void *(*append) (void *h, void *t), 
	       int node)
{
  return((node == NULL_T) ? 0 : 
	 append(append(gram_tree_map(gram, new, append, gram_leftchild(gram, node)), 
		       new(gram, node)), 
		(gram_rightflag(gram, node) != PARENT) ? 
		gram_tree_map(gram, new, append, gram_rightnode(gram, node)) : 0));
}

void *
gram_print_node (const grammar_t *gram, int treenode)
{
  err_debug("%s%s ", gram_label(gram, treenode), gram_subscript(gram, treenode));
  return(0);
}

void *
gram_null_append (void *h, void *t)
{
  return(0);
}

void *
gram_terminal (const grammar_t *gram, int treenode)
{
  return((gram_is_terminalnode(gram,treenode)) ?
	 (void *) nodelist_new(treenode) : 0);
}

void *
gram_terminal_or_foot (const grammar_t *gram, int treenode)
{
  return((gram_is_anchornode(gram,treenode) || 
	  gram_is_footnode(gram,treenode) || 
	  gram_is_terminalnode(gram,treenode)) ?
	 (void *) nodelist_new(treenode) : 0);
}

void *
nodelist_new_wrap (const grammar_t *gram, int treenode)
{
  return((void *) nodelist_new(treenode));
}

void *
nodelist_app_wrap (void *h, void *t)
{
  return((void *) nodelist_append((nodelist_t *) h, (nodelist_t *) t));
}
