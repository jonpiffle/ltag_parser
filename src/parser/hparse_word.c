
/* hparse_word - management of storage for one or more anchors per tree */

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

static char rcsid[] = "$Id: hparse_word.c,v 1.1 2000/06/25 06:42:41 anoop Exp $";

#include "mem.h"
#include "err.h"
#include "hparse.h"

hpword_t *
hpword_new (int word_index)
{
  hpword_t *new;
  new = (hpword_t *)mem_alloc(sizeof(hpword_t));
  new->w = word_index;
  new->next = 0;
  return(new);
}

void
hpword_delete (hpword_t *t)
{
  hpword_t *i, *j;
  i = t;
  while (i)
    {
      j = i;
      i = i->next;
      mem_free(j);
    }
}

int
hpword_eq (hpword_t *a, hpword_t *b)
{
  hpword_t *t, *u;
  int eqflag = 0;  /* assume a!=b */
  t = a; u = b;
  while (1)
    {
      if ((t == 0) && (u == 0)) { 
	eqflag = 1; 
	break; 
      }
      if (((t == 0) && (u != 0)) || ((t != 0) && (u == 0))) { 
	break; /* lengths are unequal */
      }
      if (t->w == u->w) { 
	t = t->next; 
	u = u->next; 
      } else { 
	break; 
      }
    }
  return(eqflag);
}

hpword_t *
hpword_append (hpword_t *al, hpword_t *bl)
{
  hpword_t *t, *a, *b;
  a = hpword_copy(al);
  b = hpword_copy(bl);
  for (t = a; t ; t = t->next)
    {
      if (t->next == 0) { 
	t->next = b; 
	return(a);
      }
    }
  return(b);
}

hpword_t *
hpword_copy (hpword_t *t)
{
  hpword_t *next, *new;
  if (t == 0) { return(0); }
  next = hpword_copy(t->next);
  new = (hpword_t *)mem_alloc(sizeof(hpword_t));
  new->w = t->w;
  new->next = next;
  return(new);
}

void
hpword_debug (hand_t *hand, hpword_t *t)
{
  hpword_t *p;
  if (t == 0) { err_debug(": nil"); }
  for (p = t; p; p = p->next)
    {
      err_debug(": [%d]%s", p->w, hand_word_at_index(hand, p->w));
      if (p == p->next) { err_abort("hpword_debug", "bad list\n"); }
    }
}

void
hpword_print (hand_t *hand, hpword_t *t)
{
  hpword_t *p;
  int flag = 0;
  if (t == 0) { err_print(HPWORD_EMPTY); return; }
  err_print(HPWORD_BEGIN);
  for (p = t; p; p = p->next)
    {
      if (flag) { err_print(HPWORD_SEPERATOR); }
      err_print("%s", hand_word_at_index(hand, p->w));
      flag = 1;
    }
  err_print(HPWORD_END);
}

void
hpword_print_feats (hand_t *hand, hpword_t *t, int tree)
{
  hpword_t *p;
  if (t == 0) { err_print("[<>]"); }
  for (p = t; p; p = p->next)
    {
      hand_print_feats(hand, p->w, tree);
    }
}

