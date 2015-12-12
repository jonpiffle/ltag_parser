
/* ia - infinite arrays */

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

static char rcsid[] = "$Id: ia.c,v 1.8 2000/10/21 17:11:51 anoop Exp $";

#ifdef USE_MEMCPY
#include <string.h>
#endif

#include "mem.h"
#include "err.h"
#include "myassert.h"
#include "ia.h"

ia_t *
ia_new (unsigned ia_blocsize)
{
  ia_t *new;
  new = (ia_t *) mem_alloc(sizeof(ia_t));
  new->ia_bloc = iabloc_new(new, ia_blocsize);
  new->ia_blocsize = ia_blocsize;
  new->ia_index = IA_EMPTY;
  return(new);
}

iabloc_t *
iabloc_new (ia_t *ia, unsigned ia_blocsize)
{
  iabloc_t *new;

  new = (iabloc_t *) mem_alloc(sizeof(iabloc_t));
  new->bloc = (void **) mem_alloc(ia_blocsize * sizeof(void *));

#ifdef USE_MEMCPY
  memset(new->bloc, 0, ia_blocsize * sizeof(void *));
#else
  {
    int i;
    for (i = 0; i < ia_blocsize; i++)
      {
	new->bloc[i] = 0;
      }
  }
#endif

  new->next = 0;
  return(new);
}

void
ia_delete (ia_t *ia, void (*delete) (void *))
{
  iabloc_t *i, *j;
  unsigned v, blocsize;

  if (ia == 0) { return; }

  i = ia_bloc(ia);
  blocsize = ia_getblocsize(ia);

  while (i)
  {
     j = i;
     i = i->next;
     for (v = 0; v < blocsize; v++)
       {
	 if (j->bloc[v]) { delete(j->bloc[v]); }
       }
     mem_free(j->bloc);
     mem_free(j);
  }
  mem_free(ia);
}

void
ia_debug (ia_t *ia, void (*debug) (void *))
{
  iabloc_t *p;
  unsigned blocsize, i;
  unsigned j = 0;

  err_debug("{%d}", ia_index(ia));
  if (ia_isempty(ia)) { 
    err_debug("<empty>"); 
    return; 
  }

  blocsize = ia_getblocsize(ia);
  for (p = ia_bloc(ia); p; p = p->next)
    {
      for (i = 0; i < blocsize; i++, j++)
      {
#ifdef DEBUG_IA
         err_debug("%d[%d]: ", j, i);
	 if (p->bloc[i]) { debug(p->bloc[i]); }
	 else { err_debug("<empty>"); }
#else
	 if (p->bloc[i]) { 
	   debug(p->bloc[i]); 
	 }
#endif
      }
    }
}

void *
ia_get (ia_t *ia, int idx)
{
  unsigned blocsize, bc;
  int i;
  iabloc_t *b;

  blocsize = ia_getblocsize(ia);
  b = ia_bloc(ia);

  if (idx < blocsize) {
    return(b->bloc[idx]);
  }

#ifdef DEBUG_IA
  err_warning("ia_get", "blocsize=%d, array size could be optimized\n", 
	      blocsize);
#endif

  bc = idx / blocsize;
  for (i = 0; i < bc; i++, b = b->next)
    {
      if (b == 0) {
	err_warning("ia_get", "no such index %d\n", idx);
	return(0);
      }
    }
  return(b->bloc[idx - (blocsize * i)]);
}

void
ia_put (ia_t *ia, int idx, void *value)
{
  unsigned blocsize, bc;
  int i;
  iabloc_t *b;

  myassert(idx > IA_EMPTY);

  if (ia_index(ia) < idx) { 
    ia->ia_index = idx;
  }

  blocsize = ia_getblocsize(ia);
  bc = idx / blocsize;

  for (i = 0, b = ia_bloc(ia); i < bc; i++, b = b->next)
    {
      if (b->next == 0) {
	b->next = iabloc_new(ia, blocsize);
      }
    }

#ifdef DEBUG_IA
  err_debug("inserting value at virtual=%d, real=%d, %d\n", 
	    idx, i, idx - (blocsize * i));
  err_debug("new ia_index=%d\n", ia_index(ia));
#endif

  b->bloc[idx - (blocsize * i)] = value;
}

void
ia_append (ia_t *ia, void *value)
{
  unsigned blocsize, bc;
  int i, idx;
  iabloc_t *b;

  blocsize = ia_getblocsize(ia);
  ia->ia_index++;
  idx = ia_index(ia);
  bc = idx / blocsize;

  for (i = 0, b = ia_bloc(ia); i < bc; i++, b = b->next)
    {
      if (b->next == 0) {
	b->next = iabloc_new(ia, blocsize);
        b = b->next;
        i++;
	break;
      }
    }

#ifdef DEBUG_IA
  err_debug("inserting value at virtual=%d, real=%d, %d\n", 
	    idx, i, idx - (blocsize * i));
#endif

  b->bloc[idx - (blocsize * i)] = value;
}

void *
ia_shrink (ia_t *ia)
{
  myassert(!ia_isempty(ia));
  ia->ia_index--;
  return(ia_get(ia, ia_index(ia)+1));
}

void
ia_ndelete (void *v)
{
  return;
}

void
ia_vdelete (void *v)
{
  if (v) { mem_free(v); }
}

void
ia_exchange (ia_t *ia, int i, int j)
{
  void *tmp;
  tmp = ia_get(ia, i);
  ia_put(ia, i, ia_get(ia, j));
  ia_put(ia, j, tmp);
}

int
ia_compare (ia_t *ia, int i, int j, int (*compare) (void *, void *))
{
  return(compare(ia_get(ia, i), ia_get(ia, j)));
}


/* 
   to compile:
   gcc -D__IA_TEST -DDEBUG_MEM -DDEBUG_IA -o iatest mem.c err.c ia.c

*/

#ifdef __IA_TEST

void
debug_func (void *x)
{
  err_debug("%s ", (char *)x);
}

int
main (int argc, char **argv)
{
  ia_t *ia;
  int i;
  char arr[12][2] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l"};


#ifdef DEBUG_MEM
  mem_debug_init();
#endif

  ia = ia_new(9);
  for (i = 0; i < 12; i++)
  {
     ia_append(ia, (void *)arr[i]);
  }

  ia_put(ia, 25, (void *)"x");
  ia_append(ia, (void *)"y");
  ia_put(ia, 27, (void *)"z");

  for (i = 0; i < 12; i++)
  {
     ia_append(ia, (void *)arr[i]);
  }

  ia_debug(ia, debug_func);
  ia_delete(ia, ia_ndelete);

#ifdef DEBUG_MEM
  mem_debug_close();
  mem_report();
#endif

  return(0);
}

#endif /* __IA_TEST */
