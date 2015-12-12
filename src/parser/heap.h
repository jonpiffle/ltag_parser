
/* heap - heap implementation using ia */

#ifndef _HEAP_H
#define _HEAP_H

#define USE_SHIFTS

#include "myassert.h"
#include "ia.h"

#ifdef	__cplusplus
extern "C" {
#endif

  typedef struct heap_s {
    ia_t *arr;
    int heap_start;
    int (*compare)(void *, void *);
  } heap_t;

#ifdef	__cplusplus
}
#endif

/* inlines */

static inline int
heap_left (int i)
{
#ifdef USE_SHIFTS
  return(i << 1);
#else
  return(2*i);
#endif
}

static inline int
heap_right (int i)
{
#ifdef USE_SHIFTS
  return((i << 1) | 0x01);
#else
  return((2*i)+1);
#endif
}

static inline int
heap_parent (int i)
{
#ifdef USE_SHIFTS
  return(i >> 1);
#else
  return(i/2);
#endif
}

static inline ia_t *
heap_arr (heap_t *heap)
{
  return(heap->arr);
}

static inline int
heap_isempty (heap_t *heap)
{
  myassert(!ia_isempty(heap->arr));
  return(ia_length(heap->arr) == 0);
}

static inline void *
heap_max (heap_t *heap)
{
  return(heap_isempty(heap) ? 0 : 
	 ia_get(heap->arr, heap->heap_start));
}

static inline int
heap_start (heap_t *heap)
{
  return(heap->heap_start);
}

static inline int
heap_length (heap_t *heap)
{
  myassert(!ia_isempty(heap->arr));
  return(ia_length(heap->arr));
}

static inline void *
heap_get (heap_t *heap, int i)
{
  return(heap_isempty(heap) ? 0 : ia_get(heap->arr, i));
}

static inline int
heap_sz (heap_t *heap)
{
  return(heap_isempty(heap) ? 0 : ia_length(heap_arr(heap)));
}

/* functions */

heap_t *heap_new (unsigned , int (*) (void *, void *));
void heapify (heap_t *, int );
void heap_insert (heap_t *, void *);
void *heap_extract (heap_t *);
void heap_debug (heap_t *, void (*) (void *));
void heap_delete (heap_t *, void (*) (void *));
void heap_printtree (heap_t *, void (*) (void *));
void heap_printnode (heap_t *, int , void (*) (void *));

#endif /* _HEAP_H */
