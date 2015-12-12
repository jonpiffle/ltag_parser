
/* hparse_add - add states into chart */

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

static char rcsid[] = "$Id: hparse_add.c,v 1.15 2001/08/22 21:08:21 anoop Exp $";

#include "myassert.h"
#include "mem.h"
#include "err.h"
#include "hparse.h"

void
hparse_add (hparse_t *hp, hand_t *hand, hpstate_t *t, hpbackptr_t *bp, ia_t *plist)
{
  heap_t *heap;
  hpstate_t *new;

#ifdef DEBUG_HPARSE_ADD
  err_debug("hparse_add:\n");
  hpstate_debug(hand, t);
#endif

  if (t->g && ((t->g->i > t->i) || (t->g->j < t->j))) {
    return;
  }

  heap = hparse_chart_get(hp, t->i, t->j);
  if (heap == 0) {

    heap = heap_new(_hp_state_heap_len, hpstate_compare);
    hparse_chart_put(hp, t->i, t->j, heap);

  } else {

    if (hp_check_heap(heap, t, bp)) {
      return;
    }
  }

  myassert(heap);
  myassert(plist);

  new = hpstate_dup(t, bp);
  new->w = hpword_copy(t->w);
  heap_insert(heap, (void *)new);
  ia_append(plist, (void *)new);
  return;
}

int
hp_check_heap (heap_t *heap, hpstate_t *t, hpbackptr_t *bp)
{
  hpstate_t *s;
  int p;

  myassert(heap);
  myassert(t->logprob == bp->logprob);

  for (p = heap_start(heap); p <= heap_length(heap); p++)
    {
      s = (hpstate_t *)heap_get(heap, p);
      myassert(s);

      if (hpstate_add_eq(s, t)) {
	hp_combine_logprob(s, bp);
	if ((t->type == init) && (bp->b || bp->c)) {
	  if (s->bp == 0) { 
	    s->bp = heap_new(_hp_backptr_heap_len, hpbackptr_compare);
	  }
	  heap_insert(s->bp, (void *)hpbackptr_new(bp->b, bp->c, bp->logprob));
	}
	return(1);
      }
    }
  return(0);
}

void
hp_combine_logprob (hpstate_t *s, hpbackptr_t *bp)
{
  heap_t *heap;
  hpstate_t *b, *c;
  double logprob;
  hpbackptr_t *p, *new;
  int i;

  b = bp->b;
  c = bp->c;
  logprob = bp->logprob;

  hp_replace_logprob(s, bp->logprob); 

  if ((b == 0) && (c == 0)) { 
    return; 
  }
  
  if ((s == b) || (s == c)) { 
    err_abort("hp_combine_logprob", 
	      "parent eq child, s=%p; b=%p; c=%p\n", s, b, c); 
  }

  if (s->bp == 0) { 
    s->bp = heap_new(_hp_backptr_heap_len, hpbackptr_compare);
  }

  heap = s->bp;
  for (i = heap_start(heap); i <= heap_length(heap); i++)
    {
      p = (hpbackptr_t *)heap_get(heap, i);
      myassert(p);

      if (hpbackptr_state_eq(p->b, b) && hpbackptr_state_eq(p->c, c))
	{
	  hp_replace_backptr_logprob(p, logprob);
	  return;
	}
    }

  new = hpbackptr_new(b, c, logprob);
  heap_insert(heap, (void *)new);
  return;
}

void
hp_replace_logprob (hpstate_t *s, double logprob)
{
  if (logprob > s->logprob) { s->logprob = logprob; }
}

void
hp_replace_backptr_logprob (hpbackptr_t *p, double logprob)
{
  if (logprob > p->logprob) { p->logprob = logprob; }
}

hpstate_t *
hp_state_exists (heap_t *heap, gnode_t n, gnode_t g, int fl, int fr)
{
  int i;
  hpstate_t *s;

  if (heap == 0) { return(0); }
  for (i = heap_start(heap); i <= heap_length(heap); i++)
    {
      s = (hpstate_t *)heap_get(heap, i);
      myassert(s);

      if ((s->n == n) &&
	  (s->pos == top) &&
	  (s->fl == fl) &&
	  (s->fr == fr) &&
	  (s->type == item))
	{
#ifdef DEBUG_HPARSE_ADD
	  err_debug("hp_state_exists: %p: span=%d,%d: heap=%p: start=%d, length=%d\n", 
		    s, s->i, s->j, heap, 
		    (heap) ? heap_start(heap) : -1, 
		    (heap) ? heap_length(heap) : -1);
#endif
	  return(s);
	}
    }
  return(0);
}

int
hp_adjroot_state_exists (hparse_t *hp, hand_t *hand, gnode_t n, int i, int j)
{
  heap_t *heap;
  hpstate_t *s;
  int p;

  heap = hparse_chart_get(hp, i, j);
  if (heap == 0) { return(0); }

  for (p = heap_start(heap); p <= heap_length(heap); p++)
    {
      s = (hpstate_t *)heap_get(heap, p);
      myassert(s);

      if ((s->n == n) &&
	  (s->pos == top) &&
	  (s->type == goal)) 
	{
	  return(1);
	}
    }
  return(0);
}

ia_t *
hp_subst_state_exists (hparse_t *hp, hand_t *hand, gnode_t n, int i, int j)
{
  heap_t *heap;
  ia_t *slist;
  hpstate_t *s;
  glabel_t label;
  int p;
  int flag;

  slist = hp->states_found;
  myassert(slist);
  ia_reset(slist);

  heap = hparse_chart_get(hp, i, j);
  if (heap == 0) { return(slist); }

  label = hand_label_index(hand, n);

  for (flag = 0, p = heap_start(heap); p <= heap_length(heap); p++)
    {
      s = (hpstate_t *)heap_get(heap, p);
      myassert(s);

      /* check if the subst node was predicted at i,j */
#ifdef DEBUG_HPARSE
      if ((s->n == n) &&
	  (s->pos == top) &&
	  (s->type == goal))
	{
	  flag = 1;
	}
#else
      flag = 1;
#endif

      if ((hand_is_subst(hand, s->n)) &&
	  (hand_label_index(hand, s->n) == label) &&
	  (s->pos == top) &&
	  (s->type == goal))
	{
	  ia_append(slist, s);
	}
    }
  if (flag == 0) { ia_reset(slist); }
  return(slist);
}

ia_t *
hp_adj_state_exists (hparse_t *hp, hand_t *hand, gnode_t n, int fl, int fr)
{
  heap_t *heap;
  int p;
  hpstate_t *s;
  glabel_t label;
  gnode_t footnode;
  int flag;
  ia_t *slist;

  slist = hp->states_found;
  myassert(slist);
  ia_reset(slist);

  heap = hparse_chart_get(hp, fl, fr);
  if (heap == 0) { return(slist); }

  footnode = hand_footnode(hand, hand_tree_from_node(hand, n));
  label = hand_label_index(hand, n);

  for (flag = 0, p = heap_start(heap); p <= heap_length(heap); p++)
    {
      s = (hpstate_t *)heap_get(heap, p);
      myassert(s);

      /* check if a footnode was predicted at fl,fr */
      if ((s->n == footnode) &&
	  (s->pos == bot) &&
	  (s->type == item) &&
	  (s->fl == fl) &&
	  (s->fr == fr))
	{
	  flag = 1;
	}

      /* if footnode exists, check for internal node */
      if ((hand_adjoinable_node(hand, s->n)) &&
	  (hand_label_index(hand, s->n) == label) &&
	  (s->pos == bot) &&
	  (s->type == item)) 
	{
	  ia_append(slist, s);
	}
    }
  if (flag == 0) { ia_reset(slist); }
  return(slist);
}

ia_t *
hp_internal_state_exists (hparse_t *hp, hand_t *hand, gnode_t n, int i, int j)
{
  heap_t *heap; 
  int p;
  hpstate_t *s;
  ia_t *slist;

  slist = hp->states_found;
  myassert(slist);
  ia_reset(slist);

  heap = hparse_chart_get(hp, i, j);
  if (heap == 0) { return(slist); }

  for (p = heap_start(heap); p <= heap_length(heap); p++)
    {
      s = (hpstate_t *)heap_get(heap, p);
      myassert(s);

      if ((s->n == n) &&
	  (s->pos == top) &&
	  (s->type == item))
	{
#ifdef DEBUG_HPARSE_ADD
	  err_debug("hp_internal_state_exists: %p: %s%s in %s: span=%d,%d: heap=%p, start=%d, length=%d\n", 
		    s, 
		    hand_label(hand, s->n),
		    hand_subscript(hand, s->n),
		    hand_treename(hand, hand_tree_from_node(hand, s->n)),
		    i, j,
		    heap,
		    (heap) ? heap_start(heap) : -1, 
		    (heap) ? heap_length(heap) : -1);
#endif
	  ia_append(slist, s);
	}
    }
  return(slist);
}

ia_t *
hp_internal_goal_exists (hparse_t *hp, hand_t *hand, gnode_t n, int i, int j)
{
  heap_t *heap; 
  int p;
  hpstate_t *s;
  ia_t *slist;

  slist = hp->states_found;
  myassert(slist);
  ia_reset(slist);

  heap = hparse_chart_get(hp, i, j);
  if (heap == 0) { return(slist); }

  for (p = heap_start(heap); p <= heap_length(heap); p++)
    {
      s = (hpstate_t *)heap_get(heap, p);
      myassert(s);

      if ((s->n == n) &&
	  (s->pos == top) &&
	  (s->type == goal) &&
	  (s->bp != 0))
	{
#ifdef DEBUG_HPARSE_ADD
	  err_debug("hp_internal_goal_exists: %s%s in %s: span=%d,%d: heap=%p, start=%d, length=%d: backptr: ", 
		    hand_label(hand, s->n),
		    hand_subscript(hand, s->n),
		    hand_treename(hand, hand_tree_from_node(hand, s->n)),
		    i, j,
		    s->bp,
		    (heap) ? heap_start(s->bp) : -1, 
		    (heap) ? heap_length(s->bp) : -1);
	  heap_debug(s->bp, hpbackptr_debug);
	  err_debug("\n");
#endif
	  ia_append(slist, s);
	}
    }
  return(slist);
}
