
/* lex_active - find all trees active for a particular sentence */

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

static char rcsid[] = "$Id: lex_active.c,v 1.1 2000/06/06 16:11:16 anoop Exp $";

#include "err.h"
#include "mem.h"
#include "spar.h"
#include "lex.h"

int *
lex_active (lex_t *l)
{
  unsigned i, k, sz;
  spar_t *s;
  int *active_tbl;
  const grammar_t *gram;

  gram = l->gram;
  s = l->lex_mtx;

  sz = gram->num_trees;
  active_tbl = (int *) mem_alloc(sz * sizeof(int));
  for (i = 0; i < sz; i++) { active_tbl[i] = 0; }

  sz  = spar_col_size(s);
  for (i = 0; i < sz; i++)
    {
      for (k = spar_row_start(s,i); k < spar_row_size(s,i); k++)
	{
	  if (lex_info_all_pruned(spar_value(s,k))) { continue; }
	  active_tbl[spar_row(s,k)] = 1;
	}
    }

  /* if non-lexicalized trees are permitted then add them all
     to the active tbl as well
  */
  if (_gram_allow_eps_trees) {
    sz = l->num_eps;
    for (i = 0; i < sz; i++)
      {
	active_tbl[gram_tree_from_node(gram, l->eps_tbl[i].treenode)] = 1;
      }
  }

  return(active_tbl);
}


void
lex_active_debug (lex_t *l)
{
  unsigned i, sz;

  sz = l->gram->num_trees;
  for (i = 0; i < sz; i++)
    {
      if (l->active_tbl[i]) {
	err_debug("active: %s\n", gram_treename(l->gram,i));
      }
    }
}
