
/* hparse_backptr - storage of backpointers while parsing */

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

static char rcsid[] = "$Id: hparse_backptr.c,v 1.4 2000/10/10 19:22:16 anoop Exp $";

#include "mem.h"
#include "err.h"
#include "hparse.h"

hpbackptr_t *
hpbackptr_new (hpstate_t *b, hpstate_t *c, double logprob)
{
  hpbackptr_t *new;
  new = (hpbackptr_t *) mem_alloc(sizeof(hpbackptr_t));
  new->b = b;
  new->c = c;
  new->logprob = logprob;
  return(new);
}

int
hpbackptr_compare (void *a, void *b)
{
  hpbackptr_t *p, *q;
  p = (hpbackptr_t *)a;
  q = (hpbackptr_t *)b;
  if (p->logprob > q->logprob) { return(1); }
  if (p->logprob == q->logprob) { return(0); }
  return(-1);
}

void
hpbackptr_debug (void *p)
{
  hpbackptr_t *t;
  if (p == 0) { err_debug(": <empty>"); return; }
  t = (hpbackptr_t *)p;
  err_debug(": (%p,%p,%lf)", t->b, t->c, t->logprob);
}

void
hpbackptr_delete (void *p)
{
  hpbackptr_t *t;
  if (p) {
    t = (hpbackptr_t *)p;
    mem_free(t);
  }
}

int
hpbackptr_state_eq (hpstate_t *a, hpstate_t *b)
{
  if ((a == 0) && (b == 0)) { return(1); }
  if ((a == 0) || (b == 0)) { return(0); }
  return(hpstate_backptr_eq(a, b));
}


