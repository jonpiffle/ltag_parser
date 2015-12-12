
/* heap - heap implementation using ia */

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

static char rcsid[] = "$Id: heap.c,v 1.10 2000/12/15 18:57:49 anoop Exp $";

#include <stdio.h>
#include <stdlib.h>

#include "mem.h"
#include "err.h"
#include "heap.h"

heap_t *
heap_new (unsigned sz, int (*compare) (void *, void *))
{
  heap_t *new;
  new = (heap_t *)mem_alloc(sizeof(heap_t));
  new->arr = ia_new(sz);
  ia_append(new->arr, 0);
  new->heap_start = 1;
  new->compare = compare;
  return(new);
}

void
heapify (heap_t *heap, int i)
{
  int true = 1;
  ia_t *arr = heap_arr(heap);
  int sz = ia_length(arr); 
  int l, r;
  int largest;

  while (true)
    {
      l = heap_left(i);
      r = heap_right(i);
      if ((l <= sz) && (ia_compare(arr, l, i, heap->compare) > 0))
	{
	  largest = l;
	} else {
	  largest = i;
	}
      if ((r <= sz) && (ia_compare(arr, r, largest, heap->compare) > 0))
	{
	  largest = r;
	}
      if (largest != i) 
	{
	  ia_exchange(arr, i, largest);
	  i = largest;
	} else {
	  break;
	}
    }
}

void
heap_insert (heap_t *heap, void *key)
{
  ia_t *arr = heap_arr(heap);
  int i, parent;
  void *parent_key;

  ia_append(arr, 0);
  i = ia_length(arr);
  parent = heap_parent(i);
  parent_key = ia_get(arr, parent);
  while ((i > heap_start(heap)) && 
	 ((heap->compare)(parent_key, key) < 0))
    {
      ia_put(arr, i, parent_key);
      i = parent;
      parent = heap_parent(i);
      parent_key = ia_get(arr, parent);
    }
  ia_put(arr, i, key);
}

void *
heap_extract (heap_t *heap)
{
  void *max;
  ia_t *arr = heap_arr(heap);
  int sz = ia_length(arr); 
  int start = heap_start(heap);

  if (sz < start) {
    err_abort("heap_extract", "heap underflow\n");
  }
  max = ia_get(arr, start);
  ia_exchange(arr, start, sz);
  ia_shrink(arr);
  heapify(heap, start);
  return(max);
}

void
heap_debug (heap_t *heap, void (*debug) (void *))
{
  if (heap == 0) { err_debug(": nil"); return; }
  ia_debug(heap->arr, debug);
}

void
heap_printtree (heap_t *heap, void (*prfunc) (void *))
{
  if (heap == 0) { return; }
  err_debug("\n");
  heap_printnode (heap, 1, prfunc);
  err_debug("\n");
}

void
heap_printnode (heap_t *heap, int i, void (*prfunc) (void *))
{
  int sz = heap_length(heap);
  if (i > sz) { return; }
  err_debug("( ");
  prfunc(heap_get(heap, i));
  heap_printnode(heap, heap_left(i), prfunc);
  heap_printnode(heap, heap_right(i), prfunc);
  err_debug(") ");
}

void
heap_delete (heap_t *heap, void (*delete) (void *))
{
  if (heap == 0) { return; }
  ia_delete(heap->arr, delete);
  mem_free(heap);
}

/* to compile:
   gcc -O -Wall -Winline -Wshadow -Wwrite-strings -Wcast-qual -Wcast-align \
       -D__HEAP_TEST -DUSE_ASSERT -DDEBUG_MEM -o heaptest -finline-functions \
       mem.c err.c ia.c heap.c
*/

#ifdef __HEAP_TEST

#define NUM_TRIALS 1
#define NUM_TEST 14
#define BLOC_SZ  5

void
debug_func (void *x)
{
  err_debug("%s ", (char *)x);
}


int
main (int argc, char **argv)
{
  int i, j, sz;
  heap_t *heap;
  char arr[NUM_TEST][2] = { "a", "f", "l", "e", "b", "d", "g", "i", "c", "j", "k", "f", "z", "b"};

#ifdef DEBUG_MEM
  mem_debug_init();
#endif

  heap = heap_new(BLOC_SZ, (void *)strcmp);
  
  for (j = 0; j < NUM_TRIALS; j++)
    {
      for (i = 0; i < ((NUM_TEST - j) > 0 ? (NUM_TEST - j) : NUM_TEST); i++)
	{
	  err_debug("inserting: %s\n", arr[i]);
	  heap_insert(heap, (void *)arr[i]);
	  heap_printtree(heap, debug_func);
	}

      heap_debug(heap, debug_func);

      sz = heap_sz(heap);
      printf("\n");
      for (i = 1; i <= sz; i++)
	{
	  printf("max: %s; ", (char *)heap_max(heap));
	  printf("[%d]: %s\n", i, (char *)heap_extract(heap));
	}
    }

  heap_delete(heap, ia_ndelete);

#ifdef DEBUG_MEM
  mem_debug_close();
  mem_report();
#endif

  return(0);

}

#endif /* __HEAP_TEST */
