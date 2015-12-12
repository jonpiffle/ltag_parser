
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

#ifndef SPAR_H
#define SPAR_H

/*
  Palimpsest of an idea in Programming Pearls, Jon Bentley, p96.

               0  1  2
              -       -
           0 | 4  0  8 |
           1 | 9  0  0 |
           2 | 0  6  7 |
  A matrix    -       -   of size 3x3 is represented as follows:

  | value | = | row | = rr = c * r * load

  Where load represents a load factor between 0 and 1, the
  lower the value of the load factor the sparser the matrix.

  | col | = c + 1 to maintain the following invariant:

  Column i of the matrix is always represented between 
  col[i] and col[i+1]-1

  M(i,j) is represented as:

  value -> [ 4 , 8 , 9 , 6 , 7 ]
    row -> [ 0 , 2 , 0 , 1 , 2 ]     (j)
             ^       ^   ^       ^
             |       |   |       |
             |   ,---'  ,|       |
             |   |     |         |
             |   |   '-' ,-------'
             |   |   |   |
    col -> [ 0 , 2 , 3 , 5 ]
             0   1   2        (i)

*/

#ifdef  __cplusplus
extern "C" {
#endif

  typedef struct spar_s {
    unsigned c;                     /* real length of column */
    unsigned r;                     /* real length of each row */
    unsigned rr;                    /* sparse length of all rows */
    unsigned incr;                  /* increment in case of overflow */
    int *col;
    int *row;
    void **value;
  } spar_t;


#ifdef  __cplusplus
	   }
#endif

/* spar */

extern spar_t *spar_new    (unsigned , unsigned , float , unsigned );
extern int     spar_put    (spar_t *, int , int , void *);
extern void   *spar_get    (spar_t *, int , int );
extern void    spar_debug  (spar_t *);
extern void    spar_delete (spar_t *);

/* inlines */

static inline unsigned
spar_col_size (spar_t *s)
{
  return(s->c);
}

static inline int
spar_row_start (spar_t *s, int i)
{
  return(s->col[i]);
}

static inline int
spar_row_size (spar_t *s, int i)
{
  return(s->col[i+1]);
}

static inline void *
spar_value (spar_t *s, int k)
{
  return(s->value[k]);
}

static inline void *
spar_value_at_row (spar_t *s, int j, int k)
{
  return((s->row[k] == j) ? s->value[k] : 0);
}

static inline int
spar_row (spar_t *s, int k)
{
  return(s->row[k]);
}

/* defines */

/* 
 #define spar_col_size(s)         (s->c)
 #define spar_row_start(s,i)      (s->col[i])
 #define spar_row_size(s,i)       (s->col[i+1])
 #define spar_value(s,k)          (s->value[k])
 #define spar_value_at_row(s,j,k) ((s->row[k] == j) ? s->value[k] : 0)
 #define spar_row(s,k)            (s->row[k])
*/

#endif /* SPAR_H */

