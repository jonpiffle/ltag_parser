
/* lex_prune - pruning trees selected for a sentence */

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

static char rcsid[] = "$Id: lex_prune.c,v 1.1 2000/06/06 16:11:55 anoop Exp $";

#include "err.h"
#include "sent.h"
#include "gram.h"
#include "spar.h"
#include "lex.h"

void
lex_prune (lex_t *lex)
{
  unsigned i, k, j, l;
  spar_t *s;
  leafinfo_t *i1, *i2;
  int sz, tree;
  const grammar_t *gram;

  gram = lex->gram;
  s = lex->lex_mtx;
  sz = spar_col_size(s);

  for (i = 0; i < sz; i++)
    {
      for (k = spar_row_start(s,i); k < spar_row_size(s,i); k++)
	{
	  tree = spar_row(s,k);
	  for (i1 = spar_value(s,k); i1; i1 = i1->next)
	    {
	      for (j = i+1; j < sz; j++)
		{
		  for (l = spar_row_start(s,j); l < spar_row_size(s,j); l++)
		    {
		      if (spar_row(s,l) != tree) { continue; }
		      for (i2 = spar_value(s,l); i2; i2 = i2->next)
			{
			  if (i1->pruned && i2->pruned) { continue; }
			  if (gram_treenode_cmp(gram, tree, i1->treenode, i2->treenode) > 0) { 
			    lex_prune_check(lex, tree, i1, i2, i, j);
			  }
			}
		    }
		}
	    }
	}
    }
}

void
lex_prune_tree (lex_t *l, int tree)
{
  unsigned i, k;
  spar_t *s;
  leafinfo_t *info;
  int sz;

  s = l->lex_mtx;
  sz = spar_col_size(s);

#ifdef DEBUG_LEX
  err_debug("lex_prune_tree: pruning %s\n", gram_treename(l->gram,tree));
#endif

  for (i = 0; i < sz; i++)
    {
      for (k = spar_row_start(s,i); k < spar_row_size(s,i); k++)
	{
	  if (spar_row(s,k) == tree) {
	    for (info = spar_value(s,k); info; info = info->next)
	      {
		info->pruned = 1;
	      }
	  }
	}
    }
}

void
lex_prune_check (lex_t *l, int tree, leafinfo_t *i1, leafinfo_t *i2, 
		   int left, int right)
{
  unsigned i, k, c;
  spar_t *s;
  leafinfo_t *info;
  int treenode;
  const grammar_t *gram;
  int sz;

  s = l->lex_mtx;
  gram = l->gram;

  c = 0;

  sz = spar_col_size(s);
  treenode = i1->treenode;
  for (i = right+1; i < sz; i++)
    {
      for (k = spar_row_start(s,i); k < spar_row_size(s,i); k++)
	{
	  if (spar_row(s,k) != tree) { continue; }
	  for (info = spar_value(s,k); info; info = info->next)
	    {
	      if (info->pruned) { continue; }
	      if (info->treenode == treenode) { 
		c = 1;
		break;
	      }
	    }
	}
    }
  if (!c) { i2->pruned = 1; }

  c = 0;
  treenode = i2->treenode;
  for (i = 0; i < left; i++)
    {
      for (k = spar_row_start(s,i); k < spar_row_size(s,i); k++)
	{
	  if (spar_row(s,k) != tree) { continue; }
	  for (info = spar_value(s,k); info; info = info->next)
	    {
	      if (info->pruned) { continue; }
	      if (info->treenode == treenode) { 
		c = 1;
		break;
	      }
	    }
	}
    }
  if (!c) { i1->pruned = 1; }

#ifdef DEBUG_LEX
  err_debug("lex_prune_check: %s at %s(%d)-%s(%d): %s%s=%d; %s%s=%d\n", 
	    gram_treename(gram, tree), 
	    sent_word_at_index(l->sent, left), left,
	    sent_word_at_index(l->sent, right), right,
	    gram_label(gram, i1->treenode),
	    gram_subscript(gram, i1->treenode), i1->pruned,
	    gram_label(gram, i2->treenode),
	    gram_subscript(gram, i2->treenode), i2->pruned);
#endif

}

int
lex_info_all_pruned (leafinfo_t *info)
{
  leafinfo_t *i;
  for (i = info; i; i = i->next)
    {
      if (i->pruned == 0) return(0);
    }
  return(1);
}

