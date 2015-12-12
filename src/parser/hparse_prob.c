
/* hparse_prob - interface to prob for hparse */

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

static char rcsid[] = "$Id: hparse_prob.c,v 1.9 2000/10/03 23:49:59 anoop Exp $";

#include <strings.h>
#include "myassert.h"
#include "mem.h"
#include "err.h"
#include "hparse.h"

double
hparse_attach_prob (hand_t *hand, gnode_t bn, hpstate_t *b, hpstate_t *s)
{
  gtree_t btree, tree;
  char *bpostag, *postag;
  int bhw, hw;
  gnode_t n;
  double logpr;

  n = s->n;
  bpostag = b->postag;
  bhw = b->headword;
  postag = s->postag;
  hw = s->headword;

  btree = hand_tree_from_node(hand, bn);
  tree = hand_tree_from_node(hand, n);

#ifdef DEBUG_HPARSE_PROB
  err_debug("attach: tree'=%s, word'=%s[%d], tag'=%s", 
	    hand_treename(hand, btree), 
	    (bhw == eh) ? "-" : hand_word_at_index(hand, bhw),
	    bhw,
	    (bpostag) ? bpostag : "-");
  err_debug(", node=%s%s%s, tree=%s, word=%s[%d], tag=%s\n", 
	    hand_label(hand, n), 
	    hand_subscript_seperator(hand, n),
	    hand_subscript(hand, n),
	    hand_treename(hand, tree),
	    (hw == eh) ? "-" : hand_word_at_index(hand, hw), 
	    hw, 
	    (postag) ? postag : "-");
#endif

  logpr = prob_attach(hand->prob, 
		      hand_treename(hand, btree), 
		      bpostag, 
		      (bhw == eh) ? "-" : hand_word_at_index(hand, bhw),
		      n, 
		      hand_treename(hand, tree), 
		      postag, 
		      (hw == eh) ? "-" : hand_word_at_index(hand, hw));
  return(logpr);
}

double
hparse_NA_prob (hand_t *hand, hpstate_t *s)
{
  gnode_t n; 
  int hw;
  char *postag;
  gtree_t tree;
  double logpr;

  n = s->n;
  hw = s->headword;
  postag = s->postag;
  tree = hand_tree_from_node(hand, n);

#ifdef DEBUG_HPARSE_PROB
  err_debug("NA: node=%s%s%s, tree=%s, word=%s[%d], tag=%s\n", 
	    hand_label(hand, n), 
	    hand_subscript_seperator(hand, n),
	    hand_subscript(hand, n),
	    hand_treename(hand, tree),
	    (hw == eh) ? "-" : hand_word_at_index(hand, hw),
	    hw, 
	    (postag) ? postag : "-");
#endif

  logpr = prob_NA(hand->prob, 
		  n, 
		  hand_treename(hand, tree), 
		  postag, 
		  (hw == eh) ? "-" : hand_word_at_index(hand, hw));
  return(logpr);
}

double
hparse_prior_prob (hand_t *hand, gnode_t n, int hw, char *postag)
{
  gtree_t tree;
  double logpr;

  tree = hand_tree_from_node(hand, n);

#ifdef DEBUG_HPARSE_PROB
  err_debug("Prior: tree=%s, word=%s[%d], tag=%s\n", 
	    hand_treename(hand, tree),
	    (hw == eh) ? "-" : hand_word_at_index(hand, hw),
	    hw, 
	    (postag) ? postag : "-");
#endif

  logpr = prob_prior(hand->prob, 
		     hand_treename(hand, tree), 
		     postag, 
		     (hw == eh) ? "-" : hand_word_at_index(hand, hw));
  return(logpr);
}

double
hparse_init_prob (hand_t *hand, int label)
{
  return(prob_init(hand->prob, label));
}
