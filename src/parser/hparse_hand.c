
/* hparse_hand - common handle for interfacing grammar and lexicon to parsers */

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

static char rcsid[] = "$Id: hparse_hand.c,v 1.14 2001/08/22 21:08:55 anoop Exp $";

#ifdef USE_MEMCPY
#include <string.h>
#endif

#include "myassert.h"
#include "mem.h"
#include "err.h"
#include "hparse_hand.h"

static nodelist_t **hand_rootnode_tbl_new (hand_t *, int );
static nodelist_t **hand_init_rootnode_tbl_new (hand_t *);
static nodelist_t **hand_aux_rootnode_tbl_new (hand_t *);
static void hand_rootnode_tbl_delete (hand_t *, nodelist_t **);

hand_t *
hand_new (const char *grammar_file, FILE *stream)
{
  hand_t *new;
  unsigned sz, i;
  int root; 
  int *tree_tbl;

  new = (hand_t *) mem_alloc(sizeof(hand_t));
  new->gram = gram_new(grammar_file);
  new->corpus = corpus_new(new->gram, SPACE, USCORE, LBRACKET, RBRACKET);
  new->stream = stream;
  new->sent = 0;
  new->lex = 0;
  new->in_sent = 0;

  new->init_rootnode_tbl = 0;
  new->aux_rootnode_tbl = 0;

  /* store value checking if tree is leftaux or rightaux */
  sz = gram_num_treenodes(new->gram);
  new->leftaux_tbl  = (int *)mem_alloc(sz * sizeof(int));
  new->rightaux_tbl = (int *)mem_alloc(sz * sizeof(int));
  for (i = 0; i < sz; i++)
    {
      if (gram_auxiliary_tree(new->gram, gram_tree_from_node(new->gram, i))) {
	new->leftaux_tbl[i] = gram_leftaux_tree(new->gram, i);
	new->rightaux_tbl[i] = gram_rightaux_tree(new->gram, i);
      } else {
	new->leftaux_tbl[i] = 0;
	new->rightaux_tbl[i] = 0;
      }
    }

  new->bin = bin_new(new->gram);

  sz = hand_num_treenodes(new);
  new->headcorner_tbl = (int *) mem_alloc(sz * sizeof(int));

#ifdef USE_MEMCPY
  memset(new->headcorner_tbl, NULL_T, sz * sizeof(int));
#else
  for (i = 0; i < sz; i++) {
    new->headcorner_tbl[i] = NULL_T;
  }
#endif

  sz = gram_num_trees(new->gram);
  tree_tbl = new->gram->tree_tbl;
  for (i = 0; i < sz; i++)
    {
      root = tree_tbl[i];
      new->headcorner_tbl[root] = hand_find_headcorner(new, root);
    }

#ifdef USE_PROB
  new->prob = prob_new(new->gram);
#else
  new->prob = 0;
#endif

  /* allocate buffer used for processing input */
  new->sentbuf       = (char *)mem_alloc(_max_sentlen * sizeof(char));
  new->nbest_linebuf = (char *)mem_alloc(_max_nbest_input_len * sizeof(char));
  new->wordbuf       = (char *)mem_alloc(_max_nbest_field_len * sizeof(char));
  new->postagbuf     = (char *)mem_alloc(_max_nbest_field_len * sizeof(char));
  new->anchorbuf     = (char *)mem_alloc(_max_nbest_field_len * sizeof(char));
  new->treenamebuf   = (char *)mem_alloc(_max_nbest_field_len * sizeof(char));

  return(new);
}

void
hand_delete (hand_t *hand)
{
  myassert(hand->lex == 0);
  myassert(hand->sent == 0);
  corpus_delete(hand->corpus);

  bin_delete(hand->bin);
  mem_free(hand->headcorner_tbl);

#ifdef USE_PROB
  prob_delete(hand->prob);
#endif

  mem_free(hand->leftaux_tbl);
  mem_free(hand->rightaux_tbl);

  mem_free(hand->sentbuf);
  mem_free(hand->nbest_linebuf);
  mem_free(hand->wordbuf );
  mem_free(hand->postagbuf);
  mem_free(hand->anchorbuf);
  mem_free(hand->treenamebuf);

  gram_delete(hand->gram);
  mem_free(hand);
}

/* returns 0 if no new sent, 1 otherwise */
int
hand_next_sent (hand_t *hand)
{
  char *status;

  if (hand->in_sent) {

    myassert(hand->lex);
    myassert(hand->sent);

    hand_rootnode_tbl_delete(hand, hand->init_rootnode_tbl);
    hand_rootnode_tbl_delete(hand, hand->aux_rootnode_tbl);

    lex_delete(hand->lex);
    sent_delete(hand->sent);

    hand->init_rootnode_tbl = 0;
    hand->aux_rootnode_tbl = 0;

    hand->sent = 0;
    hand->lex = 0;

    hand->in_sent = 0;
  }

#ifndef USE_NBEST_INPUT

  status = fgets(hand->sentbuf, _max_sentlen, hand->stream);
  if (feof(hand->stream) || (status == 0)) { 

#ifdef DEBUG_HAND
    err_warning("hand_next_sent", "end of sentence detected\n");
#endif

    return(0); 
  }

  hand->sent = sent_new(hand->corpus, hand->sentbuf);
  if (hand->sent == 0) { 

#ifdef DEBUG_HAND
    err_warning("hand_next_sent", "sent_new reported empty sentence\n");
#endif

    return(0); 
  }

  hand->lex = lex_new(hand->sent, hand->gram);
  if (hand->lex == 0) { 
    sent_delete(hand->sent);
    err_warning("hand", "failed to lexicalize sent: %s\n", hand->sentbuf);
    return(0); 
  }

#else

  {
    int idx;
    double logprob;
    char *sptr;
    int start_flag = 1;
    int linenum = 0;
    int scan_status;

    for (;;)
      {
	status = fgets(hand->nbest_linebuf, _max_nbest_input_len, hand->stream);
	if (feof(hand->stream) || (status == 0)) { 

#ifdef DEBUG_HAND
	  err_warning("hand_next_sent:nbest", "end of sentence detected\n");
#endif
	  return(0); 
	}

	linenum++;

	if ((sptr = strstr(hand->nbest_linebuf, "sent=\""))) {

	  strcpy(hand->sentbuf, &sptr[6]);
	  hand->sentbuf[strlen(hand->sentbuf)-2] = '\0';
	  hand->sent = sent_new(hand->corpus, hand->sentbuf);
	  if (hand->sent == 0) { 

#ifdef DEBUG_HAND
	    err_warning("hand_next_sent:nbest", "sent_new reported empty sentence\n");
#endif

	    return(0); 
	  }
	  hand->lex = lex_nbest_new(hand->sent, hand->gram);

#ifdef DEBUG_HAND
	  if ((hand->lex == 0) || (lex_mtx(hand->lex) == 0)) {
	    err_abort("hand_next_sent:nbest", "lex_nbest_new failed\n");
	  }
#endif

	  /* sentence has been read */
	  if (start_flag == 1) {
	    start_flag = 0;
	  } else {
	    err_abort("nbest input",
		      "line %d: duplicate sentence line or missing end: %s\n",
		      linenum, hand->nbest_linebuf);
	  }

	} else {

	  if (start_flag) {
	    err_abort("nbest input",
		      "line %d: sentence to parse not found: %s\n",
		      linenum, hand->nbest_linebuf);
	  }

	  if (strcmp(hand->nbest_linebuf, "end\n") == 0)
	    {
	      start_flag = 1;
	      if (_prune_first_pass) { lex_prune(hand->lex); }
	      hand->lex->active_tbl = lex_active(hand->lex);
#ifdef DEBUG_LEX
	      lex_debug(hand->lex);
#endif
	      break;
	    }
	  else {
	    scan_status = sscanf(hand->nbest_linebuf, 
				 "%d %s %s %s %s %lf\n", 
				 &idx, 
				 hand->wordbuf, 
				 hand->postagbuf, 
				 hand->anchorbuf, 
				 hand->treenamebuf, 
				 &logprob);
	    if (scan_status != 6) {
	      err_abort("nbest input",
			"line %d: invalid input for nbest input mode: %s\n",
			linenum, hand->nbest_linebuf);
	    }
	    if (lex_nbest(hand->lex, 
			  idx, 
			  hand->wordbuf, 
			  hand->anchorbuf, 
			  hand->treenamebuf, 
			  hand->postagbuf) == 0) 
	      {
		err_warning("lex_nbest", 
			    "line %d: could not add tree=%s for word=%s at index=%d\n", 
			    linenum, hand->treenamebuf, hand->wordbuf, idx);
	      }
	  }
	}
      }
  }

#endif

  hand->init_rootnode_tbl = hand_init_rootnode_tbl_new(hand);
  hand->aux_rootnode_tbl = hand_aux_rootnode_tbl_new(hand);

  hand->in_sent = 1;
  return(1);
}

anchor_t *
hand_anchors_for_tree (hand_t *hand, int tree, int node, int k, anchor_t *prev)
{
  leafinfo_t *n, *start, *p;
  spar_t *lex;

  lex = lex_mtx(hand->lex);
  p = (leafinfo_t *) prev;

  if (tree != NULL_T) {
    start = spar_get(lex, k, tree);
  } else {
    if (p == 0) {
      err_abort("hand_anchors_for_tree", 
		"internal error: previous pointer cannot be NULL\n");
    }
    start = p->next;
  }

  for (n = start; n; n = n->next)
    {
      if ((n->pruned) || (node != n->treenode)) { 
	continue; 
      } else {

#ifdef DEBUG_HAND
	err_debug("hand_anchors: %d-%s/%s = %s%s(%d)\nhand_anchors: feat=%s\n", 
		  k, 
		  sent_word_at_index(hand->sent, k), 
		  gram_treename(hand->gram, tree),
		  hand_label(hand, n->treenode), 
		  hand_subscript(hand, n->treenode), 
		  n->treenode,
		  lex_feature_value(n->feat));
#endif

	break;
      }
    }
  return((anchor_t *)n);
}

void
hand_print_feats (hand_t *hand, int i, int tree)
{
  leafinfo_t *n;
  spar_t *s;
  int eps_index;

  s = hand->lex->lex_mtx;
  err_print("[");
  for (n = spar_get(s,i,tree); n; n = n->next)
    {
      if (n->pruned) { continue; }
      if ((_gram_allow_eps_trees == 0) ||
	  ((eps_index = hand_is_eps_anchor(hand, n->treenode)) == -1)) {
	err_print("<%s>", lex_feature_value(n->feat));
      } else {
	err_print("<%s>", lex_eps_feature_value(hand->lex, eps_index));
      }
    }
  err_print("]");
}

nodelist_t **
hand_rootnode_tbl_new (hand_t *hand, int treetype)
{
  const grammar_t *gram;
  nodelist_t **newtbl;
  unsigned num_labels;
  glabel_t label;
  nodelist_t *new, *head;
  int k;

  num_labels = gram_num_labels(hand->gram);
  newtbl = (nodelist_t **) mem_alloc (num_labels * sizeof(nodelist_t *));

  for (label = 0; label < num_labels; label++)
    {
      head = 0;
      gram = hand->gram;

      for (k = labeltree_start(gram,label); 
	   k != NULL_T; 
	   k = labeltree_next(gram,k))
	{
	  if (lex_is_active(hand->lex, k) && 
	      (gram_tree_type(gram, k) == treetype)) 
	    {
	      new = nodelist_new(hand_rootnode(hand, k));
	      if (head) { nodelist_append(head, new); }
	      else { head = new; }
	    }
	}
      newtbl[label] = head;
    }
  return(newtbl);
}

nodelist_t **
hand_init_rootnode_tbl_new (hand_t *hand)
{
  return(hand_rootnode_tbl_new(hand, INIT_TREE));
}

nodelist_t **
hand_aux_rootnode_tbl_new (hand_t *hand)
{
  return(hand_rootnode_tbl_new(hand, AUX_TREE));
}

void
hand_rootnode_tbl_delete (hand_t *hand, nodelist_t **tbl)
{
  unsigned num_labels;
  glabel_t label;

  num_labels = gram_num_labels(hand->gram);
  for (label = 0; label < num_labels; label++)
    {
      if (tbl[label]) {
	nodelist_delete(tbl[label]);
      }
    }
  mem_free(tbl);
}

nodelist_t *
hand_init_rootnodes_top (hand_t *hand, const char *label_string)
{
  glabel_t label;

  label = gram_label_from_string(hand->gram, label_string);
  if (label == NULL_LABEL) {
    err_abort("hand_rootnodes_for_label", 
	      "cannot find label: %s\n", label);
  }
  return(hand_init_rootnodes(hand, label));
}

nodelist_t *
hand_init_rootnodes (hand_t *hand, int label)
{
  return(hand->init_rootnode_tbl[label]);
}

nodelist_t *
hand_aux_rootnodes (hand_t *hand, int label)
{
  return(hand->aux_rootnode_tbl[label]);
}

void
hand_nodelist_delete (nodelist_t *nl)
{
  nodelist_delete(nl);
}

int
hand_left_sibling (hand_t *hand, gnode_t node)
{
  gnode_t ln;
  ln = hand_leftnode(hand, node);
  return((ln == node) ? NULL_T : ln);
}

int
hand_right_sibling (hand_t *hand, gnode_t node)
{
  gnode_t n;
  n = hand_rightnode(hand, node);
  return((hand_rightflag(hand, n) == PARENT) ? NULL_T : n);
}

int
hand_find_headcorner (hand_t *new, int root)
{
  const grammar_t *gram;
  int node, leftchild, hc, hctype, new_hc, new_hctype;

  hc = UNK_HEAD;
  hctype = IS_INTERNAL;

  leftchild = hand_leftchild(new, root);
  
  /* terminal node */
  if (leftchild == NULL_T) {
    new->headcorner_tbl[root] = root;
    return(root);
  }

  gram = new->gram;
  /* n-ary branching */
  for (node = leftchild; 
       node != root;
       node = hand_rightnode(new, node))
    {
      new_hc = hand_find_headcorner(new, node);
      new_hctype = hand_nodetype(new, new_hc);

      myassert(new_hctype != IS_INTERNAL);
      new->headcorner_tbl[node] = new_hc;

      if (hand_cmp_headcorner(hctype, new_hctype) > 0) {
	hc = new_hc;
	hctype = new_hctype;
      }
    }

  myassert(hctype != IS_INTERNAL);
  new->headcorner_tbl[root] = hc;
  return(hc);
}

/* hand_cmp_headcorner
   is used to pick a new hc in the following order:

   1. footnode
   2. anchor
   3. terminal
   4. subst
   5. epsilon

   given above order, hand_cmp_headcorner returns:
     < 0  if hc should be replaced by new_hc
     > 0  if hc should not be replaced by new_hc
     == 0 if hc is equal to new_hc

   when hand_cmp_headcorner returns 0, hc is not replaced by new_hc to
   get leftmost hc, hc is replaced by new_hc to get rightmost hc.  
*/

int
hand_cmp_headcorner (int hctype, int new_hctype)
{
  if (hctype == IS_INTERNAL) { return(1); }

  if ((hctype == IS_FOOT) && (new_hctype == IS_FOOT)) { 
    err_warning("hand_cmp_headcorner", "two footnodes detected in a tree\n"); 
  }

  if (hctype == new_hctype) { return(0); }

  switch(hctype) {
  case IS_FOOT:
    break;
  case IS_ANCHOR:
    if (new_hctype == IS_FOOT) 
      { 
	return(1); 
      }
    break;
  case IS_TERMINAL:
    if ((new_hctype == IS_FOOT) ||
	(new_hctype == IS_ANCHOR))
      { 
	return(1); 
      }
    break;
  case IS_SUBST:
    if ((new_hctype == IS_FOOT) ||
	(new_hctype == IS_ANCHOR) ||
	(new_hctype == IS_TERMINAL))
      { 
	return(1); 
      }
    break;
  case IS_EPS:
    if ((new_hctype == IS_FOOT) ||
	(new_hctype == IS_ANCHOR) ||
	(new_hctype == IS_SUBST) ||
	(new_hctype == IS_TERMINAL))
      { 
	return(1); 
      }
    break;
  default:
    err_abort("hand_cmp_headcorner", "unknown type %d\n", hctype);
    break;
  }
  return(-1);
}

