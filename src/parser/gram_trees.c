
/* gram_trees - functions to access the tree database */

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

static char rcsid[] = "$Id: gram_trees.c,v 1.1 2000/06/06 16:06:50 anoop Exp $";

#include <string.h>
#include "myassert.h"
#include "gram.h"
#include "err.h"

int
gram_treeindex (const grammar_t *gram, const char *treename)
{
  int i, sz;

  sz = gram->num_trees;

  for (i = 0; i < sz; i++)
    {
      if (strcmp(gram->treename_tbl[i], treename) == 0) {
	return(i);
      }
    }
  return(NULL_T);
}

int
gram_familyindex (const grammar_t *gram, const char *family)
{
  int i, sz;

  sz = gram->num_families;

  for (i = 0; i < sz; i++)
    {
      if (strcmp(gram->family_tbl[i], family) == 0) {
	return(i);
      }
    }
  return(NULL_T);
}

int
gram_index_from_label (const grammar_t *gram, const char *label)
{
  int i, sz;
  sz = gram_num_labels(gram);
  for (i = 0; i < sz; i++)
    {
      if (strcmp(label, gram->label_tbl[i]) == 0) { return(i); }
    }
  return(-1);
}

const char *
gram_subscript (const grammar_t *gram, int treenode)
{
  int subscript;
  subscript = gram_subscript_index(gram, treenode);
  return((subscript == NULL_SUBSCRIPT) ? "" : gram->subscript_tbl[subscript]);
}

int
gram_treenode_cmp (const grammar_t *gram, int tree, int node1, int node2)
{
  if (node1 == node2) { return(0); }
  return(gram_treenode_cmp_helper(gram, gram->tree_tbl[tree], node1, node2));
}

int
gram_treenode_cmp_helper (const grammar_t *gram, int treenode, int node1, int node2)
{
  int cmp;

  if (treenode == NULL_T) { return(0); }
  if (treenode == node1) { return(-1); }
  if (treenode == node2) { return(1); }
  cmp = gram_treenode_cmp_helper(gram, gram->treenode_tbl[treenode].leftchild, node1, node2);
  if (cmp) { return(cmp); }
  if (gram_rightflag(gram, treenode) != PARENT) { 
    cmp = gram_treenode_cmp_helper(gram, gram->treenode_tbl[treenode].right, node1, node2); 
    if (cmp) { return(cmp); }
  }
  return(0);
}

int
gram_is_epsilon (const grammar_t *gram, const char *key)
{
  int i, sz;

  sz = gram_num_eps(gram);
  for (i = 0; i < sz; i++)
    {
      if (strcmp(key, gram_empty_elt(gram, i)) == 0) return(1);
    }
  return(0);
}

int
gram_aux_tree_has_yield (const grammar_t *gram, int tree)
{
  nodelist_t *terminals;

  if (gram_auxiliary_tree(gram, tree) && (gram->num_subst_tbl[tree] == 0)) {

    terminals = (nodelist_t *) gram_tree_map(gram, &gram_terminal, &nodelist_app_wrap, 
					     gram_rootnode(gram, tree));
    if (!terminals) { return(0); } 
    else { nodelist_delete(terminals); }
  }
  return(1);
}

int
gram_leftaux_tree (const grammar_t *gram, int node)
{
  myassert(node != NULL_T);
  if (gram_leftchild(gram,node) == NULL_T) {
    return(gram_is_footnode(gram,node));
  } else {
    return(gram_leftaux_tree(gram,gram_rightmost_child(gram,node)));
  }
}

int
gram_rightaux_tree (const grammar_t *gram, int node)
{
  myassert(node != NULL_T);
  if (gram_leftchild(gram,node) == NULL_T) {
    return(gram_is_footnode(gram,node));
  } else {
    return(gram_rightaux_tree(gram,gram_leftchild(gram,node)));
  }
}

int
gram_parent (const grammar_t *gram, int node)
{
  int n;
  myassert (node != NULL_T);
  for (n = node; gram_rightflag(gram,n) != PARENT; n = gram_rightnode(gram, n))
    {
    }
  return(gram_rightnode(gram,n));
}

int
gram_rightmost_child (const grammar_t *gram, int node)
{
  int n, lc;
  myassert (node != NULL_T);
  if ((lc = gram_leftchild(gram, node)) == NULL_T) { return(NULL_T); }
  for (n = lc; gram_rightflag(gram,n) != PARENT; n = gram_rightnode(gram, n))
    {
    }
  return(n);
}

int
gram_leftnode (const grammar_t *gram, int node)
{
  int n, l, parent;
  parent = gram_parent(gram, node);
  if (parent == node) { return(node); } /* root node */
  for (n = l = gram_leftchild(gram,parent); n != node; l = n, n = gram_rightnode(gram,n))
    {
    }
  return(l);
}

