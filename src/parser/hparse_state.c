
/* hparse_state - management of states for the parser */

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

static char rcsid[] = "$Id: hparse_state.c,v 1.12 2000/10/14 04:50:10 anoop Exp $";

#include <strings.h>
#include "myassert.h"
#include "mem.h"
#include "err.h"
#include "hparse.h"

hpstate_t *
hpstate_new 
(
 gnode_t n, 
 hpstate_t *g,
 hppos_e pos, 
 int i, 
 int j,
 int fl, 
 int fr, 
 hpstype_e type, 
 double logprob,
 int headword,
 hpword_t *w,
 char *postag,
 hpstate_t *b,
 hpstate_t *c
 )
{
  hpstate_t *new;
  hpbackptr_t *p;

  new = (hpstate_t *)mem_alloc(sizeof(hpstate_t));
  new->n = n;
  new->g = g;
  new->pos = pos;
  new->i = i;
  new->j = j;
  new->fl = fl;
  new->fr = fr;
  new->type = type;
  new->logprob = logprob;
  new->headword = headword;
  new->w = w;
  new->postag = postag;

  if ((b == 0) && (c == 0)) {

    new->bp = 0;

  } else {

    new->bp = heap_new(_hp_backptr_heap_len, hpbackptr_compare);
    p = hpbackptr_new(b, c, logprob);
    heap_insert(new->bp, (void *)p);
  }

  return(new);
}

hpstate_t *
hpstate_dup (hpstate_t *t, hpbackptr_t *bp)
{
  myassert(t->logprob == bp->logprob);
  return(hpstate_new
	 (
	  t->n, 
	  t->g, 
	  t->pos, 
	  t->i, 
	  t->j, 
	  t->fl, 
	  t->fr,
	  t->type, 
	  t->logprob, 
	  t->headword, 
	  t->w, 
	  t->postag,
	  bp->b, 
	  bp->c
	 ));
}


int
hpstate_compare (void *a, void *b)
{
  hpstate_t *p, *q;

  p = (hpstate_t *)a;
  q = (hpstate_t *)b;

  /* prefer items over goal or init states */
  if ((p->type == item) && (q->type != item)) 
    return(1);
  if ((p->type != item) && (q->type == item))
    return(-1);
  if ((p->type != item) && (q->type != item))
    return(0);

  /* if both are items then check logprob */
  if (p->logprob > q->logprob) 
    return(1);
  if (p->logprob < q->logprob) 
    return(-1);

  /* if the logprob values are equal then check to see if one has a
     bigger span than the other */
  if ((p->j - p->i) > (q->j - q->i))
    return(1);

  /* else states are equal */
  return(0);
}

void
hpstate_delete (void *p)
{
  hpstate_t *s;
  if (p) {
    s = (hpstate_t *)p;
    hpword_delete(s->w);
    heap_delete(s->bp, hpbackptr_delete);
    mem_free(s);
  }
}

void
hpstate_debug (hand_t *hand, hpstate_t *s)
{
  myassert(s);

  if (hand_tree_from_node(hand, s->n) == NULL_T) { return; }

  err_debug("hpstate:%p: ", s);
  hpstate_debug_state_content(hand, s);
  err_debug(" backptr");
  heap_debug(s->bp, hpbackptr_debug);
  err_debug(" words");
  hpword_debug(hand, s->w);
  err_debug("\n");
}

void
hpstate_debug_state_content (hand_t *hand, hpstate_t *s)
{
  err_debug("%s: %s%s(%d), hc=%s%s, %s%s(%d):%d,%d, %s, %d, %d, %d, %d, %s: logprob=%lf",
	    hand_treename(hand, hand_tree_from_node(hand, s->n)),
	    hand_label(hand, s->n),
	    hand_subscript(hand, s->n),
            s->n,
	    hand_label(hand, hand_headcorner(hand, s->n)),
	    hand_subscript(hand, hand_headcorner(hand, s->n)),
	    (s->g) ? hand_label(hand, s->g->n) : "<nil>",
	    (s->g) ? hand_subscript(hand, s->g->n) : "",
	    (s->g) ? s->g->n : -1,
	    (s->g) ? s->g->i : 0,
	    (s->g) ? s->g->j : 0,
	    hppos_string(s->pos),
	    s->i,
	    s->j,
	    s->fl,
	    s->fr,
	    hpstype_string(s->type),
	    s->logprob);
}


void
hpstate_print (hand_t *hand, hpstate_t *s)
{

  myassert(s->type == item);

  if (hand_tree_from_node(hand, s->n) == NULL_T) { return; }
  err_print("%d,%d:%s:%s%s,%s,%d,%d",
         s->i, s->j,
	 hand_treename(hand, hand_tree_from_node(hand, s->n)),
	 hand_label(hand, s->n),
	 hand_subscript(hand, s->n),
	 hppos_string(s->pos),
	 s->fl,
	 s->fr);
}

void
hpstate_set_type (hpstate_t *s, hpstype_e type)
{
  s->type = type;
}

int
hpstate_add_eq (hpstate_t *s, hpstate_t *t)
{
  myassert(s);
  myassert(t);

  if ((s->n == t->n) &&
      (s->pos == t->pos) &&
      (s->i == t->i) &&
      (s->j == t->j) &&
      (s->fl == t->fl) &&
      (s->fr == t->fr) &&
      ((s->g == t->g) || (s->g && t->g && (s->g->i <= t->g->i) && (s->g->j >= t->g->j))) &&
#ifdef USE_PROB
      (s->headword == t->headword) &&
      (hpstate_postagcmp(s->postag, t->postag) == 0) &&
#endif
      (((s->type != item) && (t->type != item)) ||
       (s->type == t->type)) &&
      hpword_eq(s->w, t->w))
    {
      return(1);
    }
  return(0);
}

int
hpstate_backptr_eq (hpstate_t *t, hpstate_t *s)
{
  myassert(t);
  myassert(s);
  myassert(t->type == s->type);
  myassert(t->type == item);

  if ((t->n == s->n) &&
      (t->pos == s->pos) &&
      (t->i == s->i) &&
      (t->j == s->j) &&
#ifdef USE_PROB
      (t->headword == s->headword) &&
      (hpstate_postagcmp(t->postag, s->postag) == 0) &&
#endif
      hpword_eq(t->w, s->w))
    {
      return(1);
    }
  return(0);
}

int
hpstate_postagcmp (char *a, char *b)
{
  if ((a == 0) || (b == 0)) {
    return(0);
  }
  return(strcmp(a, b));
}

