
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

#ifndef _HPARSE_H
#define _HPARSE_H

#include "pref.h"
#include "heap.h"
#include "hparse_hand.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* defines */

#define ef -1  /* empty foot */
#define eh -1  /* empty head word */

#define ADJOIN       0
#define UNARY_HEAD   1
#define LEFT_HEAD    2
#define RIGHT_HEAD   3

#define COMPL_UNDEF   -1
#define COMPL_INIT     0
#define COMPL_AUX      1
#define COMPL_INTERNAL 2

/* defines for hpword_print */
#define HPWORD_EMPTY     "[nil]"
#define HPWORD_BEGIN     "["
#define HPWORD_END       "]"
#define HPWORD_SEPERATOR "+"

/* structs */

  typedef enum {
    forest,
    chart,
    simple,
    cheap
  } hpoutput_e;

  typedef struct hpword_s {
    int w;
    struct hpword_s *next;
  } hpword_t;

  typedef enum {
    top,
    bot
  } hppos_e;

  typedef enum {
    init,
    goal,
    item
  } hpstype_e;

  typedef struct hpbackptr_s {
    struct hpstate_s *b;
    struct hpstate_s *c;
    double logprob;
  } hpbackptr_t;

  typedef struct hpstate_s {
    gnode_t n;
    struct hpstate_s *g;
    hppos_e pos;
    int i;        /* node span left      */
    int j;        /* node span right     */
    int fl;       /* footnode span left  */
    int fr;       /* footnode span right */
    hpstype_e type;
    double logprob;
    int headword;
    hpword_t *w;
    char *postag;
    heap_t *bp;
  } hpstate_t;

  typedef struct hparse_s {
    unsigned start;
    unsigned len;
    hpword_t *anchor;       /* storage for init_anchor */
    ia_t *states_found;     /* storage for states found in the chart during the completer */
    spar_t *chart;          /* recognition matrix: len x len */
  } hparse_t;

#ifdef  __cplusplus
}
#endif

static inline spar_t *
hparse_chart (hparse_t *hp)
{
  return(hp->chart);
}

/*
 #define hparse_chart(hp)  (hp->chart)
*/

/* hparse */
hparse_t *hparse_main (hand_t *);
void hparse_cleanup (hand_t * , hparse_t *);
void hparse (hparse_t *, hand_t *);

void hparse_init_head (hparse_t *, hand_t *, hpstate_t *, ia_t *);
void hparse_init_anchor (hparse_t *hp, hand_t *, hpstate_t *, ia_t *, gnode_t );
void hparse_init_epsilon (hparse_t *hp, hand_t *, hpstate_t *, ia_t *, gnode_t );
void hparse_init_subst (hparse_t *hp, hand_t *, hpstate_t *, ia_t *, gnode_t );
void hparse_init_foot (hparse_t *hp, hand_t *, hpstate_t *, ia_t *, gnode_t );

void hparse_move_up (hparse_t *, hand_t *, hpstate_t *, ia_t *);
void hparse_adjoin (hparse_t *, hand_t *, hpstate_t *, ia_t *);
void hparse_unary_head (hparse_t *, hand_t *, hpstate_t *, ia_t *);
void hparse_left_head (hparse_t *, hand_t *, hpstate_t *, ia_t *);
void hparse_right_head (hparse_t *, hand_t *, hpstate_t *, ia_t *);

void hparse_completer (hparse_t *, hand_t *, hpstate_t *, ia_t *);

/* hparse_utils */
heap_t *hparse_chart_get (hparse_t *, int , int );
void hparse_chart_put (hparse_t *, int , int , heap_t *);

int hparse_headtype (hand_t *, hpstate_t *);
int hparse_comptype (hand_t *, gnode_t );

void hp_proc_ia_debug (ia_t *, hand_t *);
void hp_state_heap_debug(heap_t *, hand_t *);

const char *hppos_string (hppos_e );
const char *hpstype_string (hpstype_e );

/* hparse_add */
void hparse_add (hparse_t *, hand_t *, hpstate_t *, hpbackptr_t *, ia_t *);
int hp_check_heap (heap_t *, hpstate_t *, hpbackptr_t *);
void hp_combine_logprob (hpstate_t *, hpbackptr_t *);
void hp_replace_logprob (hpstate_t *, double );
void hp_replace_backptr_logprob (hpbackptr_t *, double );

hpstate_t *hp_state_exists (heap_t *, gnode_t , gnode_t , int , int );
int hp_adjroot_state_exists (hparse_t *, hand_t *, gnode_t , int , int );

ia_t *hp_subst_state_exists (hparse_t *, hand_t *, gnode_t , int , int );
ia_t *hp_adj_state_exists (hparse_t *, hand_t *, gnode_t , int , int );
ia_t *hp_internal_state_exists (hparse_t *, hand_t *, gnode_t , int , int );
ia_t *hp_internal_goal_exists (hparse_t *, hand_t *, gnode_t , int , int );

/* hparse_state */
hpstate_t *hpstate_new 
(
 gnode_t , 
 hpstate_t * ,
 hppos_e , 
 int , 
 int , 
 int , 
 int , 
 hpstype_e , 
 double , 
 int , 
 hpword_t *, 
 char *,
 hpstate_t *, 
 hpstate_t *
);

hpstate_t *hpstate_dup (hpstate_t *, hpbackptr_t *);

int hpstate_compare (void *, void *);
void hpstate_delete (void *);
void hpstate_debug (hand_t *, hpstate_t *);
void hpstate_debug_state_content (hand_t *, hpstate_t *);
void hpstate_print (hand_t *, hpstate_t *);
void hpstate_set_type (hpstate_t *, hpstype_e );
int hpstate_add_eq (hpstate_t *, hpstate_t *);
int hpstate_backptr_eq (hpstate_t *, hpstate_t *);
int hpstate_postagcmp (char *, char *);

/* hparse_word */
hpword_t *hpword_new (int );
void hpword_delete (hpword_t *);
int hpword_eq (hpword_t *, hpword_t *);
hpword_t *hpword_append (hpword_t *, hpword_t *);
hpword_t *hpword_copy (hpword_t *);
void hpword_debug (hand_t *, hpword_t *);
void hpword_print (hand_t *, hpword_t *);
void hpword_print_feats (hand_t *, hpword_t *, int );

/* hparse_backptr */
hpbackptr_t *hpbackptr_new (hpstate_t *, hpstate_t *, double );
int hpbackptr_compare (void *, void *);
void hpbackptr_delete (void *);
void hpbackptr_debug (void *);
int hpbackptr_state_eq (hpstate_t *, hpstate_t *);

/* hparse_prob */

static inline double
hparse_beam (void)
{
  return(-6.90775527898214);    /* ln(0.001) */
/*
  return(-9.21034037197618);       ln(0.0001)
  return(-11.5129254649702);       ln(0.00001) 
*/
}

double hparse_attach_prob (hand_t *, gnode_t , hpstate_t *, hpstate_t *);
double hparse_NA_prob (hand_t *, hpstate_t *);
double hparse_prior_prob (hand_t *, gnode_t , int , char *);
double hparse_init_prob (hand_t *, int );

/* hparse_deriv */

int hparse_derivation_handler (hparse_t *, hand_t *, hpoutput_e );
int hparse_check_valid_start (hparse_t *, hand_t *, int );
int hparse_valid_start (hand_t *, hpstate_t *);
void hparse_add_init_prob (heap_t *, hand_t *);

#endif /* _HPARSE_H */
