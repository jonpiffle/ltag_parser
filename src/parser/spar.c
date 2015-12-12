
/* spar - sparse 2-dimensional arrays */

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

static char rcsid[] = "$Id: spar.c,v 1.1 2000/06/06 16:14:06 anoop Exp $";

#ifdef USE_MEMCPY
#include <string.h>
#endif

#include <stdio.h>
#include "myassert.h"
#include "mem.h"
#include "err.h"
#include "spar.h"

/* static function defs */
static void spar_expand (spar_t *);

/* local inlines */

static inline int
invalid_indices (unsigned c, unsigned r, unsigned cc, unsigned rr)
{
  return((c < 0) || (r < 0) || (c > cc) || (r > rr));
}

static inline void
spar_err (int i, int j)
{
  err_message("spar_put", "invalid indices %d, %d\n", i, j);
}

/* 
 #define invalid_indices(c,r,cc,rr) (c < 0) || (r < 0) || (c > cc) || (r > rr)
 #define spar_err(i,j)              err_message("spar_put", "invalid indices %d, %d\n", i, j)
*/

spar_t *
spar_new (unsigned c, unsigned r, float load, unsigned incr)
{
  spar_t *new;
  int rr, i;

  new = (spar_t *) mem_alloc(sizeof(spar_t));
  new->c = c;
  new->r = r;
  new->rr = rr = (unsigned) ((float) c * (float) r * ((load <= 0) ? 0.5 : load));
  new->incr = incr;

#ifdef DEBUG_SPAR
  err_warning("spar_new", "setting sparse row size to %d\n", rr);
#endif

  new->col   = (int *) mem_alloc((c+1) * sizeof(int));
  new->row   = (int *) mem_alloc(rr * sizeof(int));
  new->value = (void **) mem_alloc(rr * sizeof(void *));

  for (i = 0; i < c+1; i++) { new->col[i] = 0; }
  for (i = 0; i < rr;  i++) { new->row[i] = -1; new->value[i] = 0; }

  return(new);
}

/* 
   spar_put 
   assumes that all cols c are inserted before col c+1 
   if this condition is not satisfied it does an
   expensive row shifting operation
*/
int
spar_put (spar_t *s, int i, int j, void *value)
{
  int k, n;
  unsigned end;

  myassert(s);

  if (invalid_indices(i, j, s->c, s->r)) { 
    spar_err(i, j);
    return(0); 
  }

  /* do a spar_get loop to see if element exists 
     if it does, replace value
   */
  for (k = s->col[i]; k < s->col[i+1]; k++)
    {
      if (s->row[k] == j) { 
	s->value[k] = value; 
	return(1); 
      }
    }

  n = s->col[i+1]+1;

  if (n > s->rr) { spar_expand(s); }

  if (s->row[n-1] == -1) {

    for (k = i+1; k <= s->c; k++) { s->col[k] = n; }

  } else {

#ifdef DEBUG_SPAR
    err_warning("spar_put", "%d,%d caused a row shift\n", i, j);
#endif

    end = s->col[s->c];
    if (end == s->rr) { spar_expand(s); }

#ifdef USE_MEMCPY
    memmove(s->row+n,   s->row+n-1,   (end-(n-1)) * sizeof(int));
    memmove(s->value+n, s->value+n-1, (end-(n-1)) * sizeof(int));
#else
    for (k = end; k > n-1; k--) {
      s->row[k]   = s->row[k-1];
      s->value[k] = s->value[k-1];
    }
#endif

    for (k = i+1; k <= s->c; k++) { s->col[k] += 1; }

  }

  s->row[n-1]   = j; 
  s->value[n-1] = value;

  return(1);
}

void
spar_expand (spar_t *s)
{
  int p, new_rr;

  if (!err_be_quiet()) {
    err_warning("spar_put", "%d not enough row size. allocating %d extra\n", s->rr, s->incr); 
  }

  new_rr = s->rr + s->incr;
  s->row = (int *) realloc(s->row, new_rr * sizeof(int));    
  s->value = (void **) realloc(s->value, new_rr * sizeof(void *));

  myassert(s->row);
  myassert(s->value);

  for (p = s->rr; p < new_rr; p++) { s->row[p] = -1; s->value[p] = 0; }
  s->rr = new_rr;
}

void *
spar_get (spar_t *s, int i, int j)
{
  int k;

  myassert(s);
  if (invalid_indices(i, j, s->c, s->r)) {
    spar_err(i, j);
    return(0); 
  }

  for (k = s->col[i]; k < s->col[i+1]; k++)
    {
      if (s->row[k] == j) { return(s->value[k]); }
    }
  return(0);
}

void
spar_debug (spar_t *s)
{
  unsigned i, k;

  myassert(s);

  for (i = 0; i < s->c; i++)
    {
      for (k = s->col[i]; k < s->col[i+1]; k++)
	{
	  err_debug("M[%d][%d] = %s\n", i, s->row[k], (char *) s->value[k]);
	}
    }
}

void
spar_delete (spar_t *s)
{
  myassert(s);
  mem_free(s->col);
  mem_free(s->row);
  mem_free(s->value);
  mem_free(s);
}

/*
main ()
{
  spar_t *s;
  char value[5][2] = { "4", "8", "9", "6", "7" };

  s = spar_new(3, 3, 0.56, 1);
  printf("putting 1\n");
  spar_put(s, 2, 2, value[4]);
  spar_put(s, 1, 0, value[2]);
  spar_put(s, 0, 0, value[0]);
  spar_put(s, 2, 1, value[3]);
  spar_put(s, 0, 1, value[4]);
  spar_put(s, 2, 2, value[0]);

  spar_debug(s);
  spar_delete(s);

  s = spar_new(30, 30000, 0.0000025, 20);
  printf("putting 2\n");
  spar_put(s, 0, 0, value[0]);
  spar_put(s, 0, 1000, value[2]);
  spar_put(s, 0, 2500, value[1]);
  spar_put(s, 2, 0, value[2]);
  spar_put(s, 2, 1705, value[3]);
  spar_put(s, 2, 2130, value[3]);

  spar_debug(s);
  spar_delete(s);
}
*/
