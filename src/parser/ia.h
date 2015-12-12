
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

#ifndef _IA_H
#define _IA_H

/*
   an ia of length 4 would be arrays of length 4 in a linked list:

   [ a, b, c, d ][]-> [ e, f, g, _ ][]-|
     0  1  2  3         4  5  6  

   the length of the `array' is (ia_index = 6)+1, in this case.

*/

#define IA_EMPTY -1
#define IA_START 0

#ifdef	__cplusplus
extern "C" {
#endif

  typedef struct iabloc_s {
    void **bloc;
    struct iabloc_s *next;
  } iabloc_t;

  typedef struct ia_s {
    iabloc_t *ia_bloc;
    unsigned ia_blocsize;     /* size of each bloc */
    int ia_index;             /* index of last entry; IA_EMPTY if ia has no elements */
  } ia_t;

#ifdef	__cplusplus
}
#endif

/* inlines */

static inline int
ia_length (ia_t *ia)
{
  return(ia->ia_index);
}

static inline unsigned
ia_getblocsize (ia_t *ia)
{
  return(ia->ia_blocsize);
}

static inline iabloc_t *
ia_bloc (ia_t *ia)
{
  return(ia->ia_bloc);
}

static inline int
ia_index (ia_t *ia)
{
  return(ia->ia_index);
}

static inline int
ia_isempty (ia_t *ia)
{
  return(ia->ia_index <= IA_EMPTY);
}

static inline void
ia_reset (ia_t *ia)
{
  /* warning: using this function will simply start insertions from
     the start of the array. it does not vacate earlier insertions */
  ia->ia_index = IA_EMPTY;
}

static inline int
ia_start (ia_t *ia)
{
  return(IA_START);
}

static inline void
ia_swap (ia_t **a, ia_t **b)
{
  ia_t *tmp;
  tmp = *a;
  *a = *b;
  *b = tmp;
}

/* defines */

/*
#define ia_length(ia)        (ia->ia_index)
#define ia_blocsize(ia)      (ia->ia_blocsize)
#define ia_bloc(ia)          (ia->ia_bloc)
#define ia_index(ia)         (ia->ia_index)
#define ia_isempty(ia)       (ia->ia_index == IA_EMPTY)
#define ia_reset(ia)         (ia->ia_index = IA_EMPTY)
#define ia_start(ia)         (IA_START)
*/

/* functions */

ia_t * ia_new (unsigned );
iabloc_t *iabloc_new (ia_t *, unsigned );
void ia_delete (ia_t *, void (*) (void *));
void ia_debug (ia_t *ia, void (*) (void *));
void *ia_get (ia_t *, int );
void ia_put (ia_t *, int , void *);
void ia_append (ia_t *, void *);
void *ia_shrink (ia_t *);
void ia_ndelete (void *);
void ia_vdelete (void *);
void ia_exchange (ia_t *, int , int );
int ia_compare (ia_t *, int , int , int (*) (void *, void *));

#endif /* _IA_H */

