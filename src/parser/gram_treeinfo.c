
/* gram_treeinfo - compute helpful data about each tree in the grammar */

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

static char rcsid[] = "$Id: gram_treeinfo.c,v 1.1 2000/06/06 16:06:18 anoop Exp $";

#include <string.h>
#include "mem.h"
#include "err.h"
#include "gram.h"

void
gram_treeinfo (grammar_t *gram)
{
  unsigned sz, i, num_labels;
  int label, prev;

  sz = gram_num_trees(gram);
  num_labels = gram_num_labels(gram);

  gram->labeltree_tbl = (int *) mem_alloc(num_labels * sizeof(int));
  gram->labeltree_list_tbl = (int *) mem_alloc(sz * sizeof(int));

  for (i = 0; i < num_labels; i++) { gram->labeltree_tbl[i] = NULL_LABEL; }
  for (i = 0; i < sz; i++)
    {
      label = gram_label_index(gram, gram_rootnode(gram, i));
      prev = gram->labeltree_tbl[label];
      gram->labeltree_list_tbl[i] = (prev == NULL_LABEL) ? NULL_T : prev;
      gram->labeltree_tbl[label] = i;
    }

  gram->inverted_tree_tbl = (int *) mem_alloc(gram_num_treenodes(gram) * sizeof(int));

  gram->num_terminal_tbl = (int *) mem_alloc(sz * sizeof(int));
  gram->num_adjoinable_tbl = (int *) mem_alloc(sz * sizeof(int));
  gram->terminal_list_tbl = (int **) mem_alloc(sz * sizeof(int *));
  gram->adjoinable_list_tbl = (int **) mem_alloc(sz * sizeof(int *));

  for (i = 0; i < sz; i++)
    {
      gram_setup_treeinfo(gram, i);
    }

#ifdef DEBUG_TREEINFO
  gram_debug_treeinfo(gram);
#endif

}

int
gram_nodetype (const grammar_t *gram, int node)
{
  if (gram_is_footnode(gram, node)) {
    return(IS_FOOT);
  }
  if (gram_is_anchornode(gram, node)) {
    return(IS_ANCHOR);
  }
  if (gram_is_substnode(gram, node)) {
    return(IS_SUBST);
  }
  if (gram_is_epsilon(gram, gram_label(gram, node))) {
    return(IS_EPS);
  }
  if (gram_is_terminalnode(gram, node)) {
    return(IS_TERMINAL);
  }
  return(IS_INTERNAL);
}


int
gram_adjoinable_node (const grammar_t *gram, int node)
{
  /* check if null adjunction holds */
  if (gram_is_NAnode(gram,node)) { 
    return(0); 
  }

  /* disallow adjunction on substitution nodes, 
     epsilon nodes and terminal nodes
     setting ALLOW_FOOT_ADJOIN to a nonzero value 
     will permit adjunctions at a foot node
  */
  switch(gram_nodetype(gram,node)) {
  case IS_SUBST:
    return(0);
  case IS_EPS:
    return(0);
  case IS_TERMINAL:
    return(0);
  case IS_FOOT:
    return(_gram_allow_foot_adjoin);
  case IS_ANCHOR:
    return(_gram_allow_anchor_adjoin);
    break;
  case IS_INTERNAL:
    /* allow adjunctions on internal nodes */
    break;
  default:
    err_abort("gram_adjoinable_node", "unknown treenode type\n");
    break;
  }
  return(1);
}

void *
gram_adjoinable (const grammar_t *gram, int treenode)
{
  return((gram_adjoinable_node(gram,treenode)) ?
	 (void *) nodelist_new(treenode) : 0);
}

void
gram_setup_treeinfo (grammar_t *gram, int tree)
{
  unsigned len, i;
  nodelist_t *nl, *n;

  /* setup inverted map from treenodes to trees */
  nl = (nodelist_t *) gram_tree_map(gram, &nodelist_new_wrap, &nodelist_app_wrap,
				    gram_rootnode(gram, tree));
  for (n = nl; n; n = n->next)
    {
      gram->inverted_tree_tbl[n->treenode] = tree;
    }
  nodelist_delete(nl);

  /* setup info for terminals */
  nl = (nodelist_t *) gram_tree_map(gram, &gram_terminal, 
				   &nodelist_app_wrap, 
				   gram_rootnode(gram, tree));
  len = nodelist_length(nl);
  gram->num_terminal_tbl[tree] = len;

  if (len != 0) {
    gram->terminal_list_tbl[tree] = (int *) mem_alloc(len * sizeof(int));
    for (n = nl, i = 0; n; n = n->next, i++)
      {
	gram->terminal_list_tbl[tree][i] = n->treenode;
      }
  }
  nodelist_delete(nl);

  /* setup info for adjoinable nodes */
  nl = (nodelist_t *) gram_tree_map(gram, &gram_adjoinable, 
				   &nodelist_app_wrap, 
				   gram_rootnode(gram, tree));
  len = nodelist_length(nl);
  gram->num_adjoinable_tbl[tree] = len;

  if (len != 0) {
    gram->adjoinable_list_tbl[tree] = (int *) mem_alloc(len * sizeof(int));
    for (n = nl, i = 0; n; n = n->next, i++)
      {
	gram->adjoinable_list_tbl[tree][i] = n->treenode;
      }
  }
  nodelist_delete(nl);
}

void
gram_delete_treeinfo (grammar_t *gram)
{
  unsigned sz, i;

  mem_free(gram->labeltree_tbl);
  mem_free(gram->labeltree_list_tbl);

  sz = gram_num_trees(gram);
  for (i = 0; i < sz; i++)
    {
      if (gram->num_terminal_tbl[i]) {
	mem_free(gram->terminal_list_tbl[i]);
      }
      if (gram->num_adjoinable_tbl[i]) {
	mem_free(gram->adjoinable_list_tbl[i]);
      }
    }
  mem_free(gram->terminal_list_tbl);
  mem_free(gram->adjoinable_list_tbl);
  mem_free(gram->num_terminal_tbl);
  mem_free(gram->num_adjoinable_tbl);

  mem_free(gram->inverted_tree_tbl);
}

void
gram_debug_treeinfo (const grammar_t *gram)
{
  unsigned sz, i, num_labels;
  int j;

  num_labels = gram->num_labels;
  for (i = 0; i < num_labels; i++)
    {
      err_debug("%s:", gram_label_from_index(gram, i));
      for (j = labeltree_start(gram,i); j != NULL_T; j = labeltree_next(gram,j))
	{
	  err_debug(" %s", gram_treename(gram, j));
	}
      err_debug("\n");
    }

  sz = gram_num_trees(gram);
  for (i = 0; i < sz; i++)
    {
      err_debug("Tree: %s\n", gram_treename(gram, i));
      err_debug("Terminals:");
      for (j = 0; j < gram->num_terminal_tbl[i]; j++)
	{
	  err_debug(" %s%s", 
		    gram_label(gram, gram->terminal_list_tbl[i][j]),
		    gram_subscript(gram, gram->terminal_list_tbl[i][j]));
	}
      err_debug("\n");
      err_debug("Adjoinable Nodes:");
      for (j = 0; j < gram->num_adjoinable_tbl[i]; j++)
	{
	  err_debug(" %s%s", 
		    gram_label(gram, gram->adjoinable_list_tbl[i][j]),
		    gram_subscript(gram, gram->adjoinable_list_tbl[i][j]));
	}
      err_debug("\n");
    }
}

int
gram_getindex (int node, int sz, int *tbl)
{
  unsigned i;
  for (i = 0; i < sz; i++)
    {
      if (node == tbl[i]) { return(i); }
    }
  return(-1);
}

int
gram_anchor_getindex (const grammar_t *gram, int tree, int node)
{
  return(gram_getindex(node, gram_num_anchors(gram, tree), 
		       gram->anchor_list_tbl[tree]));
}

int
gram_terminal_getindex (const grammar_t *gram, int tree, int node)
{
  return(gram_getindex(node, gram_num_terminals(gram, tree), 
		       gram->terminal_list_tbl[tree]));
}

int
gram_subst_getindex (const grammar_t *gram, int tree, int node)
{
  return(gram_getindex(node, gram_num_subst(gram, tree), 
		       gram->subst_list_tbl[tree]));
}

int
gram_adjoinable_getindex (const grammar_t *gram, int tree, int node)
{
  return(gram_getindex(node, gram_num_adjoinable(gram, tree), 
		       gram->adjoinable_list_tbl[tree]));
}

unsigned
gram_numchildren (const grammar_t *gram, int root)
{
  int leftchild, node;
  unsigned sz;

  if (root == NULL_T) { return(0); }

  leftchild = gram_leftchild(gram, root);
  if (leftchild == NULL_T) { return(0); }

  /* n-ary branching */
  for (node = leftchild, sz = 1; 
       node != root;
       node = gram_rightnode(gram, node), sz++)
    {
    }

  return(sz-1);
}

int
gram_label_from_string (const grammar_t *gram, const char *label_string)
{
  int i;
  unsigned num_labels;

  num_labels = gram_num_labels(gram);
  for (i = 0; i < num_labels; i++)
    {
      if (strcmp(label_string, gram_label_from_index(gram, i)) == 0) {
	return(i);
      }
    }
  return(NULL_LABEL);
}
