
/* hparse - head-corner bidirectional parser vanNoord style */

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

static char rcsid[] = "$Id: hparse.c,v 1.35 2001/08/22 21:10:38 anoop Exp $";

#ifdef USE_PARSER_HEURISTICS
#include <time.h>
#endif

#include "common.h"
#include "myassert.h"
#include "mem.h"
#include "err.h"
#include "hparse.h"

#ifdef USE_PROB
#include "prob.h"
#endif

hparse_t *
hparse_main (hand_t *hand)
{
  hparse_t *new;

  if (hand_next_sent(hand) == 0) {
#ifdef DEBUG_HPARSE
    err_warning("hparse_main", "end of corpus or first pass failed\n");
#endif
    return(0);
  } else {
#ifdef DEBUG_HPARSE
    err_warning("hparse_main", "first pass succeeded\n");
#endif
  }

  new = (hparse_t *)mem_alloc(sizeof(hparse_t));

  /* create a storage space for init_anchor */
  new->anchor = hpword_new(0);  

  /* create storage space for states extracted from the chart during the
     completer step */
  new->states_found = ia_new(_hp_states_found_len);

  /* create chart */
  new->start = 0;
  new->len = hand_sent_length(hand)+1;
  new->chart = spar_new(new->len, new->len, _hpchart_mtx_load, _hpchart_mtx_incr);

  hparse(new, hand);
  return(new);
}

void
hparse_cleanup (hand_t *hand, hparse_t *hp)
{
  spar_t *s;
  unsigned sz;
  int i, k;

  s = hparse_chart(hp);
  sz = spar_col_size(s);

#ifdef DEBUG_HPARSE_CHART
  for (i = 0; i < sz; i++)
    {
      for (k = spar_row_start(s,i); k < spar_row_size(s,i); k++)
        {
	  hp_state_heap_debug((heap_t *)spar_value(s,k), hand);
        }
    }
#endif

  for (i = 0; i < sz; i++)
    {
      for (k = spar_row_start(s,i); k < spar_row_size(s,i); k++)
        {
	  heap_delete((heap_t *)spar_value(s,k), hpstate_delete);
        }
    }

  spar_delete(hp->chart);
  ia_delete(hp->states_found, ia_ndelete);
  hpword_delete(hp->anchor);
  mem_free(hp);
}

void
hparse (hparse_t *hp, hand_t *hand)
{
  nodelist_t *nl, *np;
  gnode_t n;
  unsigned len, sz, i;
  ia_t *plist, *nlist;
  hpstate_t *s;
  int idx;
  hpstate_t t;
  hpbackptr_t bp;

#ifdef USE_PARSER_HEURISTICS
  clock_t c0, c1;
  c0 = clock();
#endif

  len = hp->len;
  plist = ia_new(_hp_process_states_len);

  sz = hand_num_toplabels(hand);
  for (i = 0; i < sz; i++)
    {
      nl = hand_init_rootnodes(hand, hand_toplabel(hand, i));
      for (np = nl; np; np = np->next)
	{
	  n          = np->treenode;
	  t.n        = n;
	  t.g        = 0;
	  t.pos      = top;
	  t.i        = hp->start;
	  t.j        = len-1;
	  t.fl       = ef;
	  t.fr       = ef;
	  t.type     = init;
	  t.headword = eh;
	  t.w        = 0;
	  t.postag   = 0;
	  t.bp       = 0;
	  t.logprob  = 0;
	  bp.logprob = t.logprob;
	  bp.b       = 0;
	  bp.c       = 0;
	  hparse_add(hp, hand, &t, &bp, plist);
	}
    }

  /* set nlist to be equal in length to plist to be safe */
  nlist = ia_new(_hp_process_states_len);

  /* compute closure */
  while (!ia_isempty(plist))
    {

#ifdef DEBUG_HPARSE
      hp_proc_ia_debug(plist, hand);
#endif

      ia_reset(nlist);

      for (idx = ia_start(plist); idx <= ia_length(plist); idx++)
	{
	  s = (hpstate_t *)ia_get(plist, idx);
	  hparse_init_head(hp, hand, s, nlist);
	  hparse_move_up(hp, hand, s, nlist);
	  hparse_completer(hp, hand, s, nlist);
	}

      ia_swap(&plist, &nlist);

#ifdef USE_PARSER_HEURISTICS
      /* stop if the maximum number of start symbols have been seen
         spanning the entire input */
      if ((_hp_max_start_count > 0) && 
	  hparse_check_valid_start(hp, hand, _hp_max_start_count)) 
	{
	  if (!err_be_quiet()) {
	    err_warning("hparse", 
			"Valid start items exceeded _hp_max_start_count = %d\n", 
			_hp_max_start_count);
	  }
	  break;
	}

      c1 = clock();
      /* stop if the time taken to parse has crossed the maximum time
         allowed. this implementation uses CPU time, so the real
         elapsed time will be longer. */
      if ((_hp_max_parse_time_allowed > 0) &&
	  (((double) (c1 - c0)/CLOCKS_PER_SEC) > _hp_max_parse_time_allowed))
	{
	  if (!err_be_quiet()) {
	    err_warning("hparse", 
			"Time exceeded _hp_max_parse_time_allowed = %lf\n", 
			_hp_max_parse_time_allowed);
	  }
	  break;
	}
#endif

    }

  ia_delete(plist, ia_ndelete);
  ia_delete(nlist, ia_ndelete);
  return;
}

void 
hparse_init_head (hparse_t *hp, hand_t *hand, hpstate_t *s, ia_t *plist)
{
  gnode_t hc;

  myassert(s);

  /* precondition */
  if (s->type != init) { return; }

  hc = hand_headcorner(hand, s->n);
  switch(hand_nodetype(hand, hc)) {

  case IS_ANCHOR:
  case IS_TERMINAL:
    if (_gram_allow_eps_trees &&
	(hand_is_eps_anchor(hand, hc) != -1)) {
      hparse_init_epsilon(hp, hand, s, plist, hc);
    } else {
      hparse_init_anchor(hp, hand, s, plist, hc);
    }
    break;

  case IS_EPS:
    if (_hp_allow_eps_adjoin) {
      hparse_init_epsilon(hp, hand, s, plist, hc);
    } else {
      hparse_init_epsilon(hp, hand, s, plist, s->n);
    }
    break;

  case IS_SUBST:
    hparse_init_subst(hp, hand, s, plist, hc);
    break;

  case IS_FOOT:
    hparse_init_foot(hp, hand, s, plist, hc);
    break;

  default:
    err_abort("hparse_init_head", 
	      "unknown nodetype for node:%d\n", hc);

  }
  hpstate_set_type(s, goal);
  return;
}

void
hparse_init_anchor (hparse_t *hp, hand_t *hand, hpstate_t *s, ia_t *plist, gnode_t node)
{
  anchor_t *j;
  gtree_t tree;
  int k;
  hpstate_t t;
  hpbackptr_t bp;

  tree = hand_tree_from_node(hand, node);

  myassert(tree != NULL_T);
  for (k = s->i; k < s->j; k++)
    {
      for (j = hand_anchors_for_tree(hand, tree, node, k, 0); 
	   j; 
	   j = hand_anchors_for_tree(hand, NULL_T, node, k, j))
	{
	  hp->anchor->w = k;

	  t.n        = j->treenode;
	  t.g        = s;
	  t.pos      = bot;
	  t.i        = k;
	  t.j        = k+1;
	  t.fl       = ef;
	  t.fr       = ef;
	  t.type     = item;
#ifdef USE_PROB
	  t.headword = k;
#else
	  t.headword = eh;
#endif
	  t.w        = hp->anchor;
	  t.postag   = j->postag;
	  t.bp       = 0;
	  t.logprob  = 0;
	  bp.logprob = t.logprob;
	  bp.b       = 0;
	  bp.c       = 0;
	  hparse_add(hp, hand, &t, &bp, plist);
	}
    }
  return;
}

void
hparse_init_epsilon (hparse_t *hp, hand_t *hand, hpstate_t *s, ia_t *plist, gnode_t node)
{
  hpstate_t t;
  hpbackptr_t bp;

  if (s->i == s->j) {
    t.n        = node;
    t.g        = s;
    t.pos      = top;
    t.i        = s->i;
    t.j        = s->i;
    t.fl       = ef;
    t.fr       = ef;
    t.type     = item;
    t.headword = s->headword;
    t.w        = 0;
    t.postag   = s->postag;
    t.bp       = 0;
    t.logprob  = 0;
    bp.logprob = t.logprob;
    bp.b       = 0;
    bp.c       = 0;
    hparse_add(hp, hand, &t, &bp, plist);
  }
  return;
}

void
hparse_init_subst (hparse_t *hp, hand_t *hand, hpstate_t *s, ia_t *plist, gnode_t node)
{
  hpstate_t *b;
  heap_t *heap;
  nodelist_t *nl, *np;
  gnode_t n;
  hpstate_t t;
  hpbackptr_t bp;
  int i, j;
  int start, end;

  start = s->i;
  end   = s->j;

  nl = hand_init_rootnodes(hand, hand_label_index(hand, node));
  for (np = nl; np; np = np->next)
    {
      n = np->treenode;
      for (i = start; i <= end; i++)
	for (j = end; j >= i; j--)
	  {
	    /* if subtree contains only subst node then only predict
               (start,end) span */
	    /* XXX: for some mysterious reason, commenting out the
               following check changes the number of derivations
               produced. this should not happen. */
	    if ((s->n == node) && (i != start) && (j != end)) { 
	      i = end+1;
	      j = start-1;
	      continue; 
	    }

	    /* disallow empty subst */
	    if ((_gram_allow_eps_trees == 0) && (i == j)) { continue; }

	    heap = hparse_chart_get(hp, i, j);
	    if ((b = hp_state_exists(heap, n, n, ef, ef))) {
	      t.n        = node;
	      t.g        = s;
	      t.pos      = top;
	      t.i        = i;
	      t.j        = j;
	      t.fl       = ef;
	      t.fr       = ef;
	      t.type     = item;
	      t.headword = s->headword;
	      t.w        = 0;
	      t.postag   = s->postag;
	      t.bp       = 0;
#ifdef USE_PROB
	      t.logprob  = b->logprob + 
		hparse_attach_prob(hand, n, b, s);
#else
	      t.logprob  = 0;
#endif
	      bp.logprob = t.logprob;
	      bp.b       = b;
	      bp.c       = 0;
	      hparse_add(hp, hand, &t, &bp, plist);

	    } else {

	      t.n        = node;
	      t.g        = s;
	      t.pos      = top;
	      t.i        = i;
	      t.j        = j;
	      t.fl       = ef;
	      t.fr       = ef;
	      t.type     = goal;
	      t.headword = s->headword;
	      t.w        = 0;
	      t.postag   = s->postag;
	      t.bp       = 0;
	      t.logprob  = 0;
	      bp.logprob = t.logprob;
	      bp.b       = 0;
	      bp.c       = 0;
	      if (s->n == node) 
		s->g = s;
	      else
		hparse_add(hp, hand, &t, &bp, plist);

	      t.n        = n;
	      t.g        = 0;
	      t.type     = init;
	      t.headword = eh;
	      t.postag   = 0;
	      hparse_add(hp, hand, &t, &bp, plist);
	    }
	  }
    }
  return;
}

void
hparse_init_foot (hparse_t *hp, hand_t *hand, hpstate_t *s, ia_t *plist, gnode_t node)
{
  int fl, fr;
  hpstate_t t;
  hpbackptr_t bp;

  fl = s->fl;
  fr = s->fr;

  myassert(fl != ef);
  myassert(fr != ef);

  t.n        = node;
  t.g        = s;
  t.pos      = bot;
  t.i        = fl;
  t.j        = fr;
  t.fl       = fl;
  t.fr       = fr;
  t.type     = item;
  t.headword = eh;
  t.w        = 0;
  t.postag   = 0;
  t.bp       = 0;
  t.logprob  = 0;
  bp.logprob = t.logprob;
  bp.b       = 0;
  bp.c       = 0;
  hparse_add(hp, hand, &t, &bp, plist);
  return;
}

void
hparse_move_up (hparse_t *hp, hand_t *hand, hpstate_t *s, ia_t *plist)
{
  int headtype;
  myassert(s);

  /* precondition */
  if ((s->type != item) || (s->g == 0) || 
      ((s->pos == top) && (s->n == s->g->n))) 
    { 
      return; 
    }

  headtype = hparse_headtype(hand, s);

  if ((headtype == ADJOIN) && (s->pos == bot)) {

    hparse_adjoin(hp, hand, s, plist);

  } else {

    if (s->pos != top) { return; }

    switch(headtype) {

    case UNARY_HEAD:
    /* 
       P   Unary branching:
       |
       H
    */
      hparse_unary_head(hp, hand, s, plist);
      break;

    case LEFT_HEAD:
    /* 
        P   Headcorner is leftchild:
       / \
      H   R
    */
      hparse_left_head(hp, hand, s, plist);
      break;

    case RIGHT_HEAD:
    /* 
        P   Headcorner is rightchild:
       / \
      L   H
    */
      hparse_right_head(hp, hand, s, plist);
      break;

    default:
      err_abort("hparse_move_up", 
		"unknown headtype for node:%d\n", s->n);
    }
  }
  return;
}

void 
hparse_adjoin (hparse_t *hp, hand_t *hand, hpstate_t *s, ia_t *plist)
{
  hpstate_t *b;
  heap_t *heap;
  nodelist_t *nl, *np;
  int i, j;
  hpstate_t *g;
  gnode_t n;
  hpstate_t t;
  hpbackptr_t bp;
  int start, end;

  g = s->g;

  /* null adjunction */

  t.n        = s->n;
  t.g        = g;
  t.pos      = top;
  t.i        = s->i;
  t.j        = s->j;
  t.fl       = s->fl;
  t.fr       = s->fr;
  t.type     = item;
  t.headword = s->headword;
  t.w        = s->w;
  t.postag   = s->postag;
  t.bp       = 0;
#ifdef USE_PROB
  t.logprob  = s->logprob + (hand_adjoinable_node(hand, s->n)) ?
    hparse_NA_prob(hand, s) : 0;
#else
  t.logprob  = 0;
#endif
  bp.logprob = t.logprob;
  bp.b       = s;
  bp.c       = 0;
  hparse_add(hp, hand, &t, &bp, plist);

  if (!hand_adjoinable_node(hand, s->n)) { return; }

  nl = hand_aux_rootnodes(hand, hand_label_index(hand, s->n));
  for (np = nl; np; np = np->next)
    {
      n = np->treenode;

      /* predict adjunction */
      start = g->i;
      end   = g->j;
	      
      if ((_hp_disallow_leftaux_wrapping) &&
	  hand_leftaux_tree(hand, n) &&
	  hand_rightaux_tree(hand, s->n))
	{ 
	  continue;
	}

      if ((_hp_disallow_leftaux_wrapping) &&
	  hand_leftaux_tree(hand, n))
	{ 
	  end = s->j;
	}

      if ((_hp_disallow_rightaux_wrapping) &&
	  hand_rightaux_tree(hand, n) &&
	  hand_leftaux_tree(hand, s->n))
	{ 
	  continue;
	}

      if ((_hp_disallow_rightaux_wrapping) &&
	  hand_rightaux_tree(hand, n))
	{ 
	  start = s->i;
	}

      for (i = start; i <= s->i; i++)
	for (j = end; j >= s->j; j--)
	  {
	    if ((i == s->i) && (j == s->j)) { continue; }
	    heap = hparse_chart_get(hp, i, j);

	    if ((b = hp_state_exists(heap, n, n, s->i, s->j))) {

	      t.n        = s->n;
	      t.g        = g;
	      t.pos      = (_hp_multiple_adjunctions) ? bot : top;
	      t.i        = i;
	      t.j        = j;
	      t.fl       = s->fl;
	      t.fr       = s->fr;
	      t.type     = item;
	      t.headword = s->headword;
	      t.w        = s->w;
	      t.postag   = s->postag;
	      t.bp       = 0;
#ifdef USE_PROB
	      t.logprob  = s->logprob + b->logprob + 
		hparse_attach_prob(hand, n, b, s);
#else
	      t.logprob  = 0;
#endif
	      bp.logprob = t.logprob;
	      bp.b       = b;
	      bp.c       = s;
	      hparse_add(hp, hand, &t, &bp, plist);

	    } else {

	      t.n        = n;
	      t.g        = 0;
	      t.pos      = top;
	      t.i        = i;
	      t.j        = j;
	      t.fl       = s->i;
	      t.fr       = s->j;
	      t.type     = init;
	      t.headword = eh;
	      t.w        = 0;
	      t.postag   = 0;
	      t.bp       = 0;
	      t.logprob  = 0;
	      bp.logprob = 0;
	      bp.b       = 0;
	      bp.c       = 0;
	      hparse_add(hp, hand, &t, &bp, plist);

	    }
	  }
    }
  return;
}

void 
hparse_unary_head (hparse_t *hp, hand_t *hand, hpstate_t *s, ia_t *plist)
{
  hpstate_t *g;
  gnode_t pt;
  hpstate_t t;
  hpbackptr_t bp;

  g = s->g;
  pt = hand_parent(hand, s->n);

  /* move dot up to parent */
  t.n        = pt;
  t.g        = g;
  t.pos      = bot;
  t.i        = s->i;
  t.j        = s->j;
  t.fl       = s->fl;
  t.fr       = s->fr;
  t.type     = item;
  t.headword = s->headword;
  t.w        = s->w;
  t.postag   = s->postag;
  t.bp       = 0;
  t.logprob  = s->logprob;
  bp.logprob = t.logprob;
  bp.b       = s;
  bp.c       = 0;
  hparse_add(hp, hand, &t, &bp, plist);
  return;
}

void 
hparse_left_head (hparse_t *hp, hand_t *hand, hpstate_t *s, ia_t *plist)
{
  hpstate_t *b;
  heap_t *heap;
  hpstate_t *g;
  gnode_t r, pt;
  int k;
  int is_footnode;
  hpword_t *wcache;
  hpstate_t t;
  hpbackptr_t bp;
  int start, end;
  int empty_subst;

  g = s->g;
  pt = hand_parent(hand, s->n);

  /* predict right sibling of headcorner */
  r = hand_rightnode(hand, s->n);
  is_footnode = hand_is_footnode(hand, s->n);

  start = s->j;
  end   = g->j;

  /* don't over-predict epsilon subtrees */
  if (hand_nodetype(hand, hand_headcorner(hand, r)) == IS_EPS) {
    end = start+1;
  }

  empty_subst = ((_gram_allow_eps_trees == 0) && 
		 (hand_is_subst(hand, hand_headcorner(hand, r))));

  for (k = end; k >= start; k--)
    {
      /* don't allow empty subst trees */
      if (empty_subst && (k == start)) { 
	continue;
      }

      heap = hparse_chart_get(hp, s->j, k);
      if ((b = hp_state_exists(heap, r, r, ef, ef))) {

	wcache     = hpword_append(s->w, b->w);

	t.n        = pt;
	t.g        = g;
	t.pos      = bot;
	t.i        = s->i;
	t.j        = k;
	t.fl       = s->fl;
	t.fr       = s->fr;
	t.type     = item;
	t.headword = (is_footnode) ? b->headword : s->headword;
	t.w        = wcache;
	t.postag   = (is_footnode) ? b->postag : s->postag;
	t.bp       = 0;
#ifdef USE_PROB
	t.logprob  = s->logprob + b->logprob;
#else
	t.logprob  = 0;
#endif
	bp.logprob = t.logprob;
	bp.b       = s;
	bp.c       = b;
	hparse_add(hp, hand, &t, &bp, plist);
	hpword_delete(wcache);

      } else {

	t.n        = r;
	t.g        = 0;
	t.pos      = top;
	t.i        = s->j;
	t.j        = k;
	t.fl       = ef;
	t.fr       = ef;
	t.type     = init;
	t.headword = s->headword;
	t.w        = 0;
	t.postag   = s->postag;
	t.bp       = 0;
	t.logprob  = 0;
	bp.logprob = 0;
	bp.b       = 0;
	bp.c       = s;       /* store a pointer to current state to get s->w */
	hparse_add(hp, hand, &t, &bp, plist);

      }
    }
  return;
}

void 
hparse_right_head (hparse_t *hp, hand_t *hand, hpstate_t *s, ia_t *plist)
{
  hpstate_t *b;
  heap_t *heap;
  hpstate_t *g;
  gnode_t l, pt;
  int k;
  int is_footnode;
  hpword_t *wcache;
  hpstate_t t;
  hpbackptr_t bp;
  int start, end;
  int empty_subst;

  g = s->g;
  pt = hand_parent(hand, s->n);

  /* predict left sibling of headcorner */
  l = hand_leftnode(hand, s->n);
  is_footnode = hand_is_footnode(hand, s->n);

  start = g->i;
  end   = s->i;

  /* don't over-predict epsilon subtrees */
  if (hand_nodetype(hand, hand_headcorner(hand, l)) == IS_EPS) {
    start = end;
  }

  empty_subst = ((_gram_allow_eps_trees == 0) && 
		 (hand_is_subst(hand, hand_headcorner(hand, l))));

  for (k = start; k <= end; k++)
    {

      /* don't allow empty subst trees */
      if (empty_subst && (k == end)) { 
	continue;
      }

      heap = hparse_chart_get(hp, k, s->i);
      if ((b = hp_state_exists(heap, l, l, ef, ef))) {

	wcache     = hpword_append(b->w, s->w);

	t.n        = pt;
	t.g        = g;
	t.pos      = bot;
	t.i        = k;
	t.j        = s->j;
	t.fl       = s->fl;
	t.fr       = s->fr;
	t.type     = item;
	t.headword = (is_footnode) ? b->headword : s->headword;
	t.w        = wcache;
	t.postag   = (is_footnode) ? b->postag : s->postag;
	t.bp       = 0;
#ifdef USE_PROB
	t.logprob  = s->logprob + b->logprob;
#else
	t.logprob  = 0;
#endif
	bp.logprob = t.logprob;
	bp.b       = b;
	bp.c       = s;
	hparse_add(hp, hand, &t, &bp, plist);
	hpword_delete(wcache);

      } else {

	t.n        = l;
	t.g        = 0;
	t.pos      = top;
	t.i        = k;
	t.j        = s->i;
	t.fl       = ef;
	t.fr       = ef;
	t.type     = init;
	t.headword = s->headword;
	t.w        = 0;
	t.postag   = s->postag;
	t.bp       = 0;
	t.logprob  = 0;
	bp.logprob = 0;
	bp.b       = s;       /* store a pointer to current state to get s->w */
	bp.c       = 0;
	hparse_add(hp, hand, &t, &bp, plist);
      }
    }
  return;
}

void 
hparse_completer (hparse_t *hp, hand_t *hand, hpstate_t *s, ia_t *plist)
{
  ia_t *slist; 
  int idx, k, sz;
  int is_footnode;
  gnode_t pt;
  hpstate_t *b;
  hpstate_t *sib;
  hpbackptr_t *p;
  hpword_t *wcache;
  hpstate_t t;
  hpbackptr_t bp;

  myassert(s);

  /* precondition */
  if ((s->type != item) || (s->pos != top) || 
      (s->g == 0) || (s->n != s->g->n)) 
    { 
      return; 
    }

  switch(hparse_comptype(hand, s->n)) {

  case COMPL_INIT:

    slist = hp_subst_state_exists(hp, hand, s->n, s->i, s->j);

    for (idx = ia_start(slist); idx <= ia_length(slist); idx++)
      {
	b = (hpstate_t *)ia_get(slist, idx);

	t.n        = b->n;
	t.g        = b->g;
	t.pos      = top;
	t.i        = s->i;
	t.j        = s->j;
	t.fl       = ef;
	t.fr       = ef;
	t.type     = item;
	t.headword = b->headword;
	t.w        = 0;
	t.postag   = b->postag;
	t.bp       = 0;
#ifdef USE_PROB
	t.logprob  = s->logprob + hparse_attach_prob(hand, b->n, s, b);
#else
	t.logprob  = 0;
#endif
	bp.logprob = t.logprob;
	bp.b       = s;
	bp.c       = 0;
	hparse_add(hp, hand, &t, &bp, plist);

      }
    break;

  case COMPL_AUX:

#ifdef DEBUG_HPARSE
    if (!hp_adjroot_state_exists(hp, hand, s->n, s->i, s->j)) { 
      return; 
    }
#endif

    slist = hp_adj_state_exists(hp, hand, s->n, s->fl, s->fr);

    for (idx = ia_start(slist); idx <= ia_length(slist); idx++)
      {
	b = (hpstate_t *)ia_get(slist, idx);

	t.n        = b->n;
	t.g        = b->g;
	t.pos      = (_hp_multiple_adjunctions) ? bot : top;
	t.i        = s->i;
	t.j        = s->j;
	t.fl       = b->fl;
	t.fr       = b->fr;
	t.type     = item;
	t.headword = b->headword;
	t.w        = b->w;
	t.postag   = b->postag;
	t.bp       = 0;
#ifdef USE_PROB
	t.logprob  = s->logprob + b->logprob + 
	  hparse_attach_prob(hand, b->n, s, b);
#else
	t.logprob  = 0;
#endif
	bp.logprob = t.logprob;
	bp.b       = s;
	bp.c       = b;
	hparse_add(hp, hand, &t, &bp, plist);

      }
    break;

  case COMPL_INTERNAL:
    pt = hand_parent(hand, s->n);
    myassert(pt != s->n);
    slist = hp_internal_goal_exists(hp, hand, s->n, s->i, s->j);
    if (ia_isempty(slist)) { break; }

    switch(hparse_headtype(hand, s)) {

    case LEFT_HEAD:
      for (idx = ia_start(slist); idx <= ia_length(slist); idx++)
	{
	  b = (hpstate_t *)ia_get(slist, idx);
	  myassert(b);
	  myassert(b->bp);

	  sz = heap_sz(b->bp);
	  for (k = heap_start(b->bp); k <= sz; k++)
	    {
	      p = (hpbackptr_t *)heap_get(b->bp, k);
	      sib = p->b;
	      if (sib == 0) { continue; }

	      is_footnode = hand_is_footnode(hand, sib->n);
	      wcache      = hpword_append(s->w, sib->w); 

	      t.n        = pt;
	      t.g        = sib->g;
	      t.pos      = bot;
	      t.i        = s->i;
	      t.j        = sib->j;
	      t.fl       = sib->fl;
	      t.fr       = sib->fr;
	      t.type     = item;
	      t.headword = (is_footnode) ? s->headword : sib->headword;
	      t.w        = wcache;
	      t.postag   = (is_footnode) ? s->postag : sib->postag;
	      t.bp       = 0;
#ifdef USE_PROB
	      t.logprob  = s->logprob + sib->logprob;
#else
	      t.logprob  = 0;
#endif
	      bp.logprob = t.logprob;
	      bp.b       = s;
	      bp.c       = sib;
	      hparse_add(hp, hand, &t, &bp, plist);
	      hpword_delete(wcache);
	    }
	}
      break;

    case RIGHT_HEAD:
      for (idx = ia_start(slist); idx <= ia_length(slist); idx++)
	{
	  b = (hpstate_t *)ia_get(slist, idx);
	  myassert(b);
	  myassert(b->bp);

	  sz = heap_sz(b->bp);
	  for (k = heap_start(b->bp); k <= sz; k++)
	    {
	      p = (hpbackptr_t *)heap_get(b->bp, k);
	      sib = p->c;
	      if (sib == 0) { continue; }

	      is_footnode = hand_is_footnode(hand, sib->n);
	      wcache      = hpword_append(sib->w, s->w); 

	      t.n        = pt;
	      t.g        = sib->g;
	      t.pos      = bot;
	      t.i        = sib->i;
	      t.j        = s->j;
	      t.fl       = sib->fl;
	      t.fr       = sib->fr;
	      t.type     = item;
	      t.headword = (is_footnode) ? s->headword : sib->headword;
	      t.w        = wcache;
	      t.postag   = (is_footnode) ? s->postag : sib->postag;
	      t.bp       = 0;
#ifdef USE_PROB
	      t.logprob  = s->logprob + sib->logprob;
#else
	      t.logprob  = 0;
#endif
	      bp.logprob = t.logprob;
	      bp.b       = sib;
	      bp.c       = s;
	      hparse_add(hp, hand, &t, &bp, plist);
	      hpword_delete(wcache);
	    }
	}
      break;

    default:
      err_abort("hparse_completer", 
		"cannot use completer for state: ");
      hpstate_debug(hand, s);
    }
    break;

  default:
    err_abort("hparse_completer", 
	      "cannot use completer for state: ");
    hpstate_debug(hand, s);
  }
  return;
}

