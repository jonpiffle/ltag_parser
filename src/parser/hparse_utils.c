
/* hparse_utils - useful functions for hparse */

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

static char rcsid[] = "$Id: hparse_utils.c,v 1.9 2000/06/25 06:39:29 anoop Exp $";

#include "myassert.h"
#include "mem.h"
#include "err.h"
#include "hparse.h"

heap_t *
hparse_chart_get (hparse_t *hp, int i, int j)
{
  return((heap_t *) spar_get(hp->chart, i, j));
}

void
hparse_chart_put (hparse_t *hp, int i, int j, heap_t *heap)
{
  if (spar_put(hp->chart, i, j, (void *)heap) == 0) {
    err_abort("hparse_add", "error inserting into chart at %d, %d\n", i, j); 
  }
}

int
hparse_headtype (hand_t *hand, hpstate_t *s)
{
  gnode_t node;

  if (s->pos == bot) { 
    return(ADJOIN); 
  } else {
    node = s->n;
    if (hand_is_unarynode(hand, node)) { 
      return(UNARY_HEAD); 
    }
    if (hand_is_leftnode(hand, node))  { 
      return(LEFT_HEAD); 
    }
    if (hand_is_rightnode(hand, node)) { 
      return(RIGHT_HEAD); 
    }
  }
  return(-1);
}

int
hparse_comptype (hand_t *hand, gnode_t n)
{
  gtree_t tree;
  tree = hand_tree_from_node(hand, n);

  if (tree == NULL_T) { return(COMPL_INTERNAL); }

  if (hand_rootnode(hand, tree) == n) {
    return((hand_auxiliary_tree(hand, tree)) ? COMPL_AUX : COMPL_INIT);
  } else {
    return(COMPL_INTERNAL);
  }

  return(COMPL_UNDEF);
}

void
hp_proc_ia_debug (ia_t *arr, hand_t *hand)
{
  int t;
  hpstate_t *s;

  err_debug("proc_len: %d\n", ia_length(arr)+1);
  for (t = ia_start(arr); t <= ia_length(arr); t++)
    {
      s = (hpstate_t *)ia_get(arr, t);
      err_debug("proc{%d,%d}: ", s->i, s->j);
      hpstate_debug(hand, s);
    }
}

void
hp_state_heap_debug(heap_t *heap, hand_t *hand)
{
  hpstate_t *s;
  int t;

  if (heap == 0) { 
    err_debug("empty heap...\n"); 
    return; 
  }

  err_debug("state_heaplen: start=%d, length=%d\n", 
	    heap_start(heap), heap_length(heap));

  for (t = heap_start(heap); t <= heap_length(heap); t++)
    {
      s = (hpstate_t *)heap_get(heap, t);
      err_debug("state_heap{%d,%d}: ", s->i, s->j);
      hpstate_debug(hand, s);
    }
}

const char *
hppos_string (hppos_e pos)
{
  switch (pos)
    {
    case top: 
      return("top");
      break;
    case bot:
      return("bot");
      break;
    default:
      err_abort("hppos_string", "strange position value %d", pos);
      return("");
    }
}

const char *
hpstype_string (hpstype_e type)
{
  switch (type)
    {
    case init: 
      return("init");
      break;
    case goal: 
      return("goal");
      break;
    case item:
      return("item");
      break;
    default:
      err_abort("hpstype_string", "strange type value %d", type);
      return("");
    }
}

