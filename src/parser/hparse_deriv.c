
/* hparse_deriv - output derivations after parsing */

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

static char rcsid[] = "$Id: hparse_deriv.c,v 1.16 2000/10/05 23:19:50 anoop Exp $";

#ifdef USE_MEMCPY
#include <string.h>
#endif

#include <math.h>

#include "myassert.h"
#include "mem.h"
#include "err.h"
#include "hparse.h"
#include "hash.h"

#ifdef USE_PROB
#include "prob.h"
#endif

#define HPARSE_SBUF_SZ       (1000)
#define HP_NULLSTR           "(nil)"

static void hparse_printnode (hand_t *hand, hpstate_t *s, gtree_t tree, gnode_t attach);
static void hparse_printtail (hand_t *hand, hpstate_t *s, hpstate_t *parent, gtree_t tree);

static int hparse_derivations_simple (hparse_t *hp, hand_t *hand);
static void hparse_derivations_simple_loop (hparse_t *, hand_t *, hpstate_t *);

static int hparse_print_forest (hparse_t *hp, hand_t *hand);
static void hparse_print_forest_loop (hparse_t *, hand_t *, hash_t *, hpstate_t *);

static int hparse_derivations_cheap (hparse_t *hp, hand_t *hand);
static void hparse_derivations_cheap_loop (hparse_t *hp, hand_t *hand, hpstate_t *parent, hpstate_t *s);

static void hparse_print_chart (hparse_t *hp, hand_t *hand);

/* returns 1 if no parses were found, 0 otherwise */
int
hparse_derivation_handler (hparse_t *hp, hand_t *hand, hpoutput_e output)
{
  int flag = 1;
  switch(output) {
  case forest:
    flag = hparse_print_forest(hp, hand);
    break;
  case chart:
    hparse_print_chart(hp, hand);
    flag = 0; 
    break;
  case simple:
    flag = hparse_derivations_simple(hp, hand);
    break;
  case cheap:
    flag = hparse_derivations_cheap(hp, hand);
    break;
  default:
    err_abort("hparse_derivation_handler", 
	      "unknown type of derivation output selected.\n");
  }
  return(flag);
}

void
hparse_add_init_prob (heap_t *heap, hand_t *hand)
{
  unsigned i, sz;
  glabel_t label;
  hpstate_t *s;

  sz = heap_sz(heap);
  if (heap) {
    for (i = 1; i <= sz; i++)
      {
	s = (hpstate_t *)heap_extract(heap);
	myassert(s);
	label = hand_label_index(hand, s->n);
	if (hand_is_toplabel(hand, label) != 0) {
	  s->logprob += hparse_init_prob(hand, label);
	}
	heap_insert(heap, s);
      }
  }
}

int
hparse_check_valid_start (hparse_t *hp, hand_t *hand, int start_count)
{
  heap_t *slist;
  int i;
  hpstate_t *s;
  int cnt = 0;

  slist = hparse_chart_get(hp, hp->start, (hp->len)-1);

  if (start_count <= 0) { return(0); }
  if (slist == 0) { return(0); }
  if (heap_length(slist) < start_count) { return(0); }
  for (i = heap_start(slist); i <= heap_length(slist); i++)
    {
      s = (hpstate_t *)heap_get(slist, i);
      myassert(s);
      if (hparse_valid_start(hand, s) != 0) { cnt++; }
    }
  if (cnt >= start_count) { 
    return(1);
  } else {
    return(0);
  }
}

/* used to check if the states in (0,n) 
   are valid start states: items which are
   rootnodes of initial trees with the same
   label as those in gram->toplabel_tbl
*/
int
hparse_valid_start (hand_t *hand, hpstate_t *s)
{
  gtree_t tree;

  tree = hand_tree_from_node(hand, s->n);

#ifndef USE_PROB
  if ((s->pos == top) &&
      (s->type == item) &&
      (hand_is_toplabel(hand, hand_label_index(hand, s->n))) &&
      (tree != NULL_T) &&
      (s->n == hand_rootnode(hand, tree)) &&
      (!(hand_auxiliary_tree(hand, tree))) &&
      (s->w != 0))
    {
      return(1);
    }
#else
  {
    nodelist_t *nl, *np;
    glabel_t lbl;
    unsigned i, sz;
    sz = hand_num_labels(hand);
    for (i = 0; i < sz; i++)
      {
	if (prob_toplabel_count(hand->prob, i) > 0) {
	  nl = hand_init_rootnodes(hand, i);
	  for (np = nl; np; np = np->next)
	    {
	      lbl = hand_label_index(hand, np->treenode);
	      if ((s->pos == top) &&
		  (s->type == item) &&
		  (hand_label_index(hand, s->n) == lbl) &&
		  (tree != NULL_T) &&
		  (s->n == hand_rootnode(hand, tree)) &&
		  (!(hand_auxiliary_tree(hand, tree))) &&
		  (s->w != 0))
		{
		  return(1);
		}
	    }
	}
      }
  }
#endif

  return(0);
}

void
hparse_printnode (hand_t *hand, hpstate_t *s, gtree_t tree, gnode_t attach)
{
  gnode_t root, foot;

  root = hand_rootnode(hand, tree);
  foot = hand_footnode(hand, tree);

  if ((s->pos == top) && (s->n == hand_rootnode(hand, tree))) {
    err_print("( %s", hand_treename(hand, tree));

    if (_hp_print_tnodes) {
      err_print("<%s%s%s/", hand_label(hand, root), 
         hand_subscript_seperator(hand, root),
         hand_subscript(hand, root));
      if (foot != -1) {
         err_print("%s%s%s", hand_label(hand, foot), 
            hand_subscript_seperator(hand, foot),
            hand_subscript(hand, foot));
      }
      err_print(">");
    }

    if (_hp_print_spans) { 
      err_print("[%d,%d,%d,%d]", s->i, s->j, s->fl, s->fr); 
    }

    hpword_print(hand, s->w);

    if (attach != -1) { 
      err_print("<%s%s%s> ", hand_label(hand, attach), 
         hand_subscript_seperator(hand, attach),
         hand_subscript(hand, attach)); 
    } else {
      err_print(" "); 
    }

    if (_hp_print_feats) { 
      hpword_print_feats(hand, s->w, tree); 
    }

  }
}

void
hparse_printtail (hand_t *hand, hpstate_t *s, hpstate_t *parent, gtree_t tree)
{
  if ((s->pos == top) && (s->n == hand_rootnode(hand, tree))) {
    err_print(" ) ");
  }
  if (parent == 0) { err_print("\n"); }
}

int
hparse_derivations_simple (hparse_t *hp, hand_t *hand)
{
  heap_t *slist;
  hpstate_t *s;
  int i, sz;
  int flag = 1;

  slist = hparse_chart_get(hp, hp->start, (hp->len)-1);

  if (slist) {
    sz = heap_sz(slist);
    for (i = 1; i <= sz; i++)
      {
	s = heap_extract(slist);
	if (hparse_valid_start(hand, s) == 0) { continue; }
	flag = 0;
	hparse_derivations_simple_loop(hp, hand, s);
	err_print("\n");
      }
  }
  return(flag);
}

void
hparse_derivations_simple_loop (hparse_t *hp, hand_t *hand, hpstate_t *s)
{
  int i, sz;
  hpbackptr_t *p;

  if (s == 0) { return; }
  err_print(" ( ");
  err_print("%p:", s);
  hpstate_print(hand, s);
  err_print(",");
  hpword_print(hand, s->w);
  err_print(" ");

  if (s->bp) {
    sz = heap_sz(s->bp);
    for (i = 1; i <= sz; i++) {
      p = (hpbackptr_t *)heap_extract(s->bp);
      myassert(p);
      hparse_derivations_simple_loop(hp, hand, p->b);
      hparse_derivations_simple_loop(hp, hand, p->c); 
    }
  }
  err_print(" ) ");
}

static int
hparse_dntbl_sz (int len)
{
  if (len < 12) { return(pow(2.0,(len+5))); }
  else { return(262144); }
}

int
hparse_print_forest (hparse_t *hp, hand_t *hand)
{
  heap_t *slist;
  int i;
  hpstate_t *s;
  hash_t *dntbl;
  int flag = 1;
  char sbuf[HPARSE_SBUF_SZ];
  glabel_t label;

  slist = hparse_chart_get(hp, hp->start, (hp->len)-1);
  dntbl = hash_new(hparse_dntbl_sz(hp->len));

  err_print("begin sent=\"");
  sent_print(hand->sent);
  err_print("\"\n");

  if (slist) {

#ifdef HPARSE_USE_HEAP_EXTRACT
    {
      int sz;
      sz = heap_sz(slist);
      hparse_add_init_prob(slist, hand);
      for (i = 1; i <= sz; i++)
	{
	  s = (hpstate_t *)heap_extract(slist);
	  myassert(s);
	  if (hparse_valid_start(hand, s) == 0) { continue; }
	  flag = 0;
	  sprintf(sbuf, "%p", s);
	  label = hand_label_index(hand, s->n);
	  err_print("start: %s [%s]\n", sbuf, hand_toplabel_feat(hand, label));
	  hparse_print_forest_loop(hp, hand, dntbl, s);
	  break;
	}
    }
#else
    for (i = heap_start(slist); i <= heap_length(slist); i++)
      {
	s = (hpstate_t *)heap_get(slist, i);
	myassert(s);
	if (hparse_valid_start(hand, s) == 0) { continue; }
	flag = 0;
	sprintf(sbuf, "%p", s);
	label = hand_label_index(hand, s->n);
	err_print("start: %s [%s]\n", sbuf, hand_toplabel_feat(hand, label));
	hparse_print_forest_loop(hp, hand, dntbl, s);
      }
#endif

  }

  err_print("end\n");
  hash_delete(dntbl, hash_vdelete);
  return(flag);
}

void
hparse_print_forest_loop (hparse_t *hp, 
			  hand_t *hand, 
			  hash_t *dntbl,
			  hpstate_t *s)
{
  gtree_t tree;
  gnode_t root, foot;
  hpbackptr_t *p;
  int i;
  char sbuf[HPARSE_SBUF_SZ];
  char *key;
  int is_auxtree;

  if (s == 0) { return; }

  assert(s->type == item);

  sprintf(sbuf, "%p", s);

  if (hash_lookup(dntbl, sbuf)) { 
    return; 
  } else { 
    key = mem_strdup(sbuf); 
    hash_enter(dntbl, key, key); 
  }

  tree = hand_tree_from_node(hand, s->n);
  myassert(tree != NULL_T);
  myassert(tree < gram_num_trees(hand->gram));

  err_print("%s: ", sbuf);
#ifdef HPARSE_USE_HEAP_EXTRACT
  err_print("%d//%f//", s->headword, s->logprob);
#endif
  err_print("%s", hand_treename(hand, tree));
  if (_hp_print_spans) {
    err_print("[%s%s][%d,%d,%d,%d]", 
	      hand_label(hand,s->n), 
	      hand_subscript(hand,s->n),
	      s->i, s->j, s->fl, s->fr);
  }

  /* if the terminal symbol is of type epsilon then the node label is
     the correct string value that should be printed out */
  if (hand_nodetype(hand, s->n) == IS_EPS) {
    err_print("%s%s%s%s", 
	      HPWORD_BEGIN, 
	      hand_label(hand,s->n), 
	      hand_subscript(hand,s->n),
	      HPWORD_END);
  } else {
    hpword_print(hand, s->w);
  }

  root = hand_rootnode(hand, tree);
  foot = hand_footnode(hand, tree);

  err_print("<%s%s%s/%s>", hand_label(hand, s->n), 
     hand_subscript_seperator(hand, s->n),
     hand_subscript(hand, s->n),
     (s->pos == top) ? "top" : "bot");

  err_print("<%s>", (s->postag) ? s->postag : "-");

  is_auxtree = hand_auxiliary_tree(hand, tree);
  if ((s->n == root) && (s->pos == top)) { 
    err_print((is_auxtree) ? "<auxroot>" : "<initroot>"); 
  } else if ((s->n == foot) && (s->pos == bot)) { 
    err_print("<auxfoot>"); 
  } else { 
    err_print("<internal>"); 
  }

  if (_hp_print_tnodes) {
    err_print("<%s%s%s/", hand_label(hand, root), 
       hand_subscript_seperator(hand, root),
       hand_subscript(hand, root));
    if (foot != -1) {
       err_print("%s%s%s", hand_label(hand, foot), 
          hand_subscript_seperator(hand, foot),
          hand_subscript(hand, foot));
    }
    err_print(">");
  }

  if ((_hp_print_feats) && (s->bp == 0)) { 
    hpword_print_feats(hand, s->w, tree); 
  } else {
    err_print("[]");
  }

  err_print(" ");
  if (s->bp) {
#ifdef HPARSE_USE_HEAP_EXTRACT
    {
      p = (hpbackptr_t *)heap_max(s->bp);
#else
    for (i = heap_start(s->bp); i <= heap_length(s->bp); i++) 
      {
	p = (hpbackptr_t *)heap_get(s->bp, i);
#endif
	myassert(p);
	if (p->b == 0) { strcpy(sbuf, HP_NULLSTR); }
	else { sprintf(sbuf, "%p", p->b); }
	err_print("[%s,", sbuf);
	if (p->c == 0) { strcpy(sbuf, HP_NULLSTR); }
	else { sprintf(sbuf, "%p", p->c); }
	err_print("%s]", sbuf);
      }
    }

  err_print("\n"); 

  if (s->bp) {

#ifdef HPARSE_USE_HEAP_EXTRACT
    { 
      int sz;
      sz = heap_sz(s->bp);
      for (i = 1; i <= sz; i++) 
	{
	  p = (hpbackptr_t *)heap_extract(s->bp);
	  myassert(p);
	  hparse_print_forest_loop(hp, hand, dntbl, p->b);
	  hparse_print_forest_loop(hp, hand, dntbl, p->c); 
	  break;
	}
    }
#else
    for (i = heap_start(s->bp); i <= heap_length(s->bp); i++) 
      {
	p = (hpbackptr_t *)heap_get(s->bp, i);
	myassert(p);
	hparse_print_forest_loop(hp, hand, dntbl, p->b);
	hparse_print_forest_loop(hp, hand, dntbl, p->c); 
      }
#endif

  }
}

int
hparse_derivations_cheap (hparse_t *hp, hand_t *hand)
{
  heap_t *slist;
  int i, sz;
  hpstate_t *s;
  int flag = 1;

  slist = hparse_chart_get(hp, hp->start, (hp->len)-1);
  err_print("( BEGIN ( \"");
  sent_print(hand->sent);
  err_print("\" ) )\n");

  if (slist) {
    sz = heap_sz(slist);
    for (i = 1; i <= sz; i++)
      {
	s = heap_extract(slist);
	if (hparse_valid_start(hand, s) == 0) { continue; }
	flag = 0;
	hparse_derivations_cheap_loop(hp, hand, 0, s);
      }
  }

  err_print("( END ( \"");
  sent_print(hand->sent);
  err_print("\" ) )\n");
  return(flag);
}

void
hparse_derivations_cheap_loop (hparse_t *hp, hand_t *hand, hpstate_t *parent, hpstate_t *s)
{
  gtree_t tree, ptree;
  gnode_t attach;
  hpbackptr_t *p;
  int i, sz;

  if (s == 0) { return; }

  tree = hand_tree_from_node(hand, s->n);
  myassert(tree != NULL_T);

  if (parent == 0) { attach = -1; ptree = NULL_T; }
  else { attach = parent->n; ptree = hand_tree_from_node(hand, parent->n); }

  hparse_printnode(hand, s, tree, attach);

  if (s->bp) {
    sz = heap_sz(s->bp);
    for (i = 1; i <= sz; i++) {
      p = (hpbackptr_t *)heap_extract(s->bp);
      myassert(p);
      hparse_derivations_cheap_loop(hp, hand, s, p->c);
      hparse_derivations_cheap_loop(hp, hand, s, p->b); 
      break;
    }
  }

  hparse_printtail(hand, s, parent, tree);
}

void
hparse_print_chart (hparse_t *hp, hand_t *hand)
{
  spar_t *s;
  unsigned sz;
  int i, k;
  hpstate_t *st;
  gtree_t tree;
  heap_t *t;
  int j, q, jsz, qsz;
  hpbackptr_t *p;
  char sbuf[HPARSE_SBUF_SZ];
  unsigned len;

  err_print("begin sent=\"");
  sent_print(hand->sent);
  err_print("\"\n");

  len = (hp->len)-1;
  s = hparse_chart(hp);
  sz = spar_col_size(s);

  for (i = 0; i < sz; i++)
    {
      for (k = spar_row_start(s,i); k < spar_row_size(s,i); k++)
        {
	  t = (heap_t *)spar_value(s,k);
	  if (t == 0) { continue; }

	  jsz = heap_sz(t);
	  for (j = 1; j <= jsz; j++)
	    {
	      st = (hpstate_t *)heap_extract(t);
 	      if (st == 0) { continue; }
	      if (st->type != item) { continue; }
	      sprintf(sbuf, "%p", st);
	      tree = hand_tree_from_node(hand, st->n);
	      myassert(tree != NULL_T);

	      if ((i == 0) && (k == len)) {
		err_print("start: %s\n", sbuf); 
	      }

	      err_print("%s: ", sbuf);
	      err_print("%s", hand_treename(hand, tree));
	      if (_hp_print_spans) {
		err_print("[%s%s][%d,%d,%d,%d]", 
			  hand_label(hand,st->n), 
			  hand_subscript(hand,st->n),
			  st->i, st->j, st->fl, st->fr);
	      }
	      hpword_print(hand, st->w);
	      if (_hp_print_feats) { 
		hpword_print_feats(hand, st->w, tree); 
	      }

	      err_print(" ");
	      if (st->bp) { 
		qsz = heap_sz(st->bp);
		for (q = 1; q <= qsz; q++) {
		  p = (hpbackptr_t *)heap_extract(st->bp);
		  myassert(p);
		  sprintf(sbuf, "%p", p->c);
		  err_print("[%s,", sbuf);
		  sprintf(sbuf, "%p", p->b);
		  err_print("%s]", sbuf);
		}
	      }

	      err_print("\n"); 
	    }
        }
    }
  err_print("end\n");
}
