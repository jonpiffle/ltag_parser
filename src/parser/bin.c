
/* bin - set up binary branching elementary trees */

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

static char rcsid[] = "$Id: bin.c,v 1.2 2000/10/16 04:33:12 anoop Exp $";

#ifdef USE_MEMCPY
#include <string.h>
#endif

#include <math.h>
#include "myassert.h"
#include "err.h"
#include "mem.h"
#include "gram.h"
#include "bin.h"

bin_t *
bin_new (const grammar_t *gram)
{
  bin_t *new;
  unsigned sz;

  new = (bin_t *) mem_alloc(sizeof(bin_t));
  new->gram = gram;

  sz = bin_newnodes(gram);
  sz += gram_num_treenodes(gram);

  new->bin_num_treenodes = sz;
  new->bin_treenode_tbl = (treenode_t *) mem_alloc(sz * sizeof(treenode_t));
  bin_binarize(new, gram);
  if (!err_be_quiet()) {
    err_debug("expanding to %d binary branching treenodes\n", sz);
  }

#ifdef DEBUG_BIN
  bin_pretty_print_trees(new);
#endif

  return(new);
}

void
bin_delete (bin_t *bin)
{
  mem_free(bin->bin_treenode_tbl);
  mem_free(bin);
}

unsigned
bin_newnodes (const grammar_t *gram)
{
  unsigned num_trees, num_newnodes, i = 0;

  num_trees = gram_num_trees(gram);
  num_newnodes = 0;
  for (i = 0; i < num_trees; i++)
    {
      num_newnodes += bin_tree_newnodes(gram, gram_rootnode(gram,i));
    }
  return(num_newnodes);
}

unsigned
bin_tree_newnodes (const grammar_t *gram, int root)
{
  int leftchild, node;
  unsigned sz, num_newnodes = 0;

  if (root == NULL_T) { return(0); }

  leftchild = gram_leftchild(gram, root);
  if (leftchild == NULL_T) { return(0); }

  /* n-ary branching */
  for (node = leftchild, sz = 0; 
       node != root;
       node = gram_rightnode(gram, node), sz++)
    {
      num_newnodes += bin_tree_newnodes(gram, node);
    }

  num_newnodes += (bin_binarynodes(sz) - 1);
  return(num_newnodes);
}

#define BIN_LOG_2 0.693147180559945

unsigned
bin_binarynodes (unsigned sz)
{
  if (sz <= 2) { return(1); }
  if (sz == 3) { return(2); }
  if (sz == 4) { return(3); }
  return(pow(2.0,(log(sz)/BIN_LOG_2)) - 1);
}

void
bin_binarize (bin_t *bin, const grammar_t *g)
{
  unsigned num_treenodes, last, i = 0;
  int parent, rightnode, rightmost;

  num_treenodes = gram_num_treenodes(g);

#ifdef USE_MEMCPY
  memcpy(bin->bin_treenode_tbl, g->treenode_tbl, (num_treenodes * sizeof(treenode_t)));
#else
  for (i = 0; i < num_treenodes; i++)
    {
      bin->bin_treenode_tbl[i].leftchild = g->treenode_tbl[i].leftchild;
      bin->bin_treenode_tbl[i].right = g->treenode_tbl[i].right;
      bin->bin_treenode_tbl[i].flag = g->treenode_tbl[i].flag;
      bin->bin_treenode_tbl[i].label = g->treenode_tbl[i].label;
      bin->bin_treenode_tbl[i].subscript = g->treenode_tbl[i].subscript;
    }
#endif

  last = num_treenodes;
  for (i = 0; i < num_treenodes; i++)
    {
      if (bin_rightflag(bin, i) == PARENT) { continue; }
      rightnode = bin_rightnode(bin, i);
      if (bin_rightflag(bin, rightnode) == PARENT) { continue; }

      /* i has more than one sibling */
      parent = bin_parent(bin, i);
      rightmost = bin_rightmost_child(bin, rightnode);

      bin->bin_treenode_tbl[i].right = last;
      bin->bin_treenode_tbl[rightmost].right = last;

      bin->bin_treenode_tbl[last].leftchild = rightnode;
      bin->bin_treenode_tbl[last].flag = PARENT;
      bin->bin_treenode_tbl[last].right = parent;
      bin->bin_treenode_tbl[last].label = NULL_LABEL;
      bin->bin_treenode_tbl[last].subscript = NULL_SUBSCRIPT;

      last++;
    }
  myassert(last == bin->bin_num_treenodes);
}

void
bin_debug_treenodes (const bin_t *bin)
{
  unsigned sz, i;
  sz = bin_num_treenodes(bin);
  for (i = 0; i < sz; i++)
    {
      err_debug(TREENODES_FORMAT, 
		bin_leftchild(bin, i),
		bin_rightnode(bin, i),
		bin_rightflag(bin, i),
		bin_label_index(bin, i),
		bin_subscript_index(bin, i));
    }
}

void
bin_pretty_print_trees (const bin_t *bin)
{
  const grammar_t *gram;
  unsigned sz, i;

  gram = bin->gram;
  sz = gram_num_trees(gram);
  for (i = 0; i < sz; i++)
    {
      err_debug("(%s ", gram_treename(gram, i));
      bin_pretty_print_tree(bin, gram_rootnode(bin->gram, i));
      err_debug(" )\n");
    }
}

void
bin_pretty_print_tree(const bin_t *bin, int root)
{
  int leftchild, node;

  if (root == NULL_T) { return; }
  leftchild = bin_leftchild(bin, root);

  /* leaf node */
  if (leftchild == NULL_T) {
    err_debug(" %s%s ", 
	      bin_label(bin, root), 
	      bin_subscript(bin, root));
    return;
  }

  /* internal node */
  err_debug(" (%s%s ", 
	    bin_label(bin, root), 
	    bin_subscript(bin, root));

  for (node = leftchild; 
       node != root;
       node = bin_rightnode(bin, node))
    {
      bin_pretty_print_tree(bin, node);
    }

  err_debug(" )");
}

int
bin_parent (const bin_t *bin, int node)
{
  myassert(node != NULL_T);
  if (bin_rightflag(bin, node) == PARENT) { return(bin_rightnode(bin, node)); }
  else { return(bin_parent(bin, bin_rightnode(bin, node))); }
}

int
bin_rightmost_child (const bin_t *bin, int node)
{
  int n;
  myassert(node != NULL_T);
  for (n = node; bin_rightflag(bin, n) != PARENT; n = bin_rightnode(bin, n))
    {
    }
  return(n);
}

const char *
bin_label (const bin_t *bin, int treenode)
{
  int label;
  label = bin_label_index(bin, treenode);

#ifdef DEBUG_BIN
  return((label == NULL_LABEL) ? "@" : 
	 (gram_valid_treenode(bin->gram, treenode) ? 
	   gram_label(bin->gram, treenode) :
	  "[ERROR]"));
#else
  return((label == NULL_LABEL) ? "@" : 
	 gram_label(bin->gram, treenode));
#endif
}

const char *
bin_subscript (const bin_t *bin, int treenode)
{
  int subscript;
  subscript = bin_subscript_index(bin, treenode);

#ifdef DEBUG_BIN
  return((subscript == NULL_SUBSCRIPT) ? "" : 
	 (gram_valid_treenode(bin->gram, treenode) ?
	  gram_subscript(bin->gram, treenode) :
	  "[ERROR]"));
#else
  return((subscript == NULL_SUBSCRIPT) ? "" : 
	 gram_subscript(bin->gram, treenode));
#endif
}

int
bin_nodetype (const bin_t *bin, int node)
{
  unsigned num_treenodes;

  num_treenodes = gram_num_treenodes(bin->gram);
  if (node >= num_treenodes) {
    /* node was added for binarizing trees */
    return(IS_INTERNAL);
  } else {
    return(gram_nodetype(bin->gram, node));
  }
}

int
bin_adjoinable_node (const bin_t *bin, int node)
{
  unsigned num_treenodes;

  num_treenodes = gram_num_treenodes(bin->gram);
  if (node >= num_treenodes) {
    /* node was added for binarizing trees 
       n.b: although the bin_nodetype of such nodes is IS_INTERNAL no
       adjunction is permitted 
    */
    return(0);
  } else {
    return(gram_adjoinable_node(bin->gram, node));
  }
}

int
bin_is_substnode (const bin_t *bin, int node)
{
  unsigned num_treenodes;

  num_treenodes = gram_num_treenodes(bin->gram);
  if (node >= num_treenodes) {
    /* node was added for binarizing trees */
    return(0);
  } else {
    return(gram_is_substnode(bin->gram, node));
  }
}

int
bin_is_anchornode (const bin_t *bin, int node)
{
  unsigned num_treenodes;

  num_treenodes = gram_num_treenodes(bin->gram);
  if (node >= num_treenodes) {
    /* node was added for binarizing trees */
    return(0);
  } else {
    return(gram_is_anchornode(bin->gram, node));
  }
}

int
bin_is_footnode (const bin_t *bin, int node)
{
  unsigned num_treenodes;

  num_treenodes = gram_num_treenodes(bin->gram);
  if (node >= num_treenodes) {
    /* node was added for binarizing trees */
    return(0);
  } else {
    return(gram_is_footnode(bin->gram, node));
  }
}

int
bin_is_NAnode (const bin_t *bin, int node)
{
  unsigned num_treenodes;

  num_treenodes = gram_num_treenodes(bin->gram);
  if (node >= num_treenodes) {
    /* node was added for binarizing trees */
    return(0);
  } else {
    return(gram_is_NAnode(bin->gram, node));
  }
}

int
bin_tree_from_node (const bin_t *bin, int node)
{
  unsigned num_treenodes;

  num_treenodes = gram_num_treenodes(bin->gram);
  if (node >= num_treenodes) {
    /* node was added for binarizing trees */
    return(bin_tree_from_node(bin, bin_parent(bin, node)));
  } else {
    return(gram_tree_from_node(bin->gram, node));
  }
}
