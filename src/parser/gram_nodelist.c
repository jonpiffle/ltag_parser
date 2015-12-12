
/* gram_nodelist - list of treenodes for search results */

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

static char rcsid[] = "$Id: gram_nodelist.c,v 1.1 2000/06/06 16:05:00 anoop Exp $";

#include "err.h"
#include "mem.h"
#include "gram.h"

nodelist_t *
nodelist_new (int treenode)
{
  nodelist_t *new;
  new = (nodelist_t *) mem_alloc(sizeof(nodelist_t));
  new->treenode = treenode;
  new->next = 0;
  return(new);
}

nodelist_t *
nodelist_append (nodelist_t *a, nodelist_t *b)
{
  nodelist_t *t;
  for (t = a; t ; t = t->next)
    {
      if (t->next == 0) { 
	t->next = b; 
	return(a);
      }
    }
  return(b);
}


void
nodelist_delete (nodelist_t *t)
{
  nodelist_t *i, *j;
  i = t;
  while (i)
    {
      j = i;
      i = i->next;
      mem_free(j);
    }
}

unsigned
nodelist_length (nodelist_t *t)
{
  unsigned len;
  nodelist_t *i;
  for (i = t, len = 0; i ; i = i->next, len++)
    {
    }
  return(len);
}

void
nodelist_debug (const grammar_t *gram, nodelist_t *t)
{
  nodelist_t *i;
  if (!t) { return; }
  for (i = t; i; i = i->next)
    {
      err_debug("%s(%d) ", gram_label(gram, i->treenode), i->treenode);
    }
  err_debug("\n");
}

