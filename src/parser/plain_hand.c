
/* hand - common handle for interfacing grammar and lexicon to parsers */

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

static char rcsid[] = "$Id: plain_hand.c,v 1.1 2000/06/06 16:13:19 anoop Exp $";

#ifdef USE_MEMCPY
#include <string.h>
#endif

#include "myassert.h"
#include "mem.h"
#include "err.h"
#include "hand.h"

hand_t *
hand_new (const char *grammar_file, FILE *stream)
{
  hand_t *new;

  new = (hand_t *) mem_alloc(sizeof(hand_t));
  new->gram = gram_new(grammar_file);
  new->corpus = corpus_new(new->gram, SPACE, USCORE, LBRACKET, RBRACKET);
  new->stream = stream;
  new->sent = 0;
  new->lex = 0;
  new->in_sent = 0;

  return(new);
}

void
hand_delete (hand_t *hand)
{
  myassert(hand->lex == 0);
  myassert(hand->sent == 0);
  corpus_delete(hand->corpus);
  gram_delete(hand->gram);
  mem_free(hand);
}

/* returns 0 if no new sent, 1 otherwise */
int
hand_next_sent (hand_t *hand)
{
  char sent[SENT_LEN];
  char *status;

  if (hand->in_sent) {

    myassert(hand->lex);
    myassert(hand->sent);

    lex_delete(hand->lex);
    sent_delete(hand->sent);

    hand->sent = 0;
    hand->lex = 0;
    hand->in_sent = 0;

  }

  status = fgets(sent, SENT_LEN, hand->stream);
  if (feof(hand->stream) || (status == 0)) { return(0); }

  hand->sent = sent_new(hand->corpus, sent);
  if (hand->sent == 0) { return(0); }

  hand->lex = lex_new(hand->sent, hand->gram);
  if (hand->lex == 0) { 
    sent_delete(hand->sent);
    err_warning("hand", "failed to lexicalize sent: %s\n", sent);
    return(0); 
  }

  hand->in_sent = 1;
  return(1);
}

nodelist_t *
hand_anchors (hand_t *hand, int i)
{
  nodelist_t *new, *head;
  leafinfo_t *n;
  spar_t *s;
  int k;

  head = 0;
  s = lex_mtx(hand->lex);
  for (k = spar_row_start(s,i); k < spar_row_size(s,i); k++)
    {
      for (n = spar_value(s,k); n; n = n->next)
	{
	  if (n->pruned) { continue; }

#ifdef DEBUG_HAND
	  err_debug("hand_anchors: %d-%s/%s = %s%s(%d)\n", 
		    i, 
		    sent_word_at_index(hand->sent, i), 
		    gram_treename(hand->gram, spar_row(s,k)),
		    hand_label(hand, n->treenode), 
		    hand_subscript(hand, n->treenode), 
		    n->treenode);
#endif

	  new = nodelist_new(n->treenode);
	  if (head) { nodelist_append(head, new); }
	  else { head = new; }
	}
    }
  return(head);
}

nodelist_t *
hand_anchors_for_tree (hand_t *hand, int tree, int node, int k)
{
  nodelist_t *new, *head;
  leafinfo_t *n;
  spar_t *lex;

  if (tree == NULL_T) { return(0); }

  head = 0;
  lex = hand->lex->lex_mtx;
  for (n = spar_get(lex, k, tree); n; n = n->next)
   {
      if ((n->pruned) || (node != n->treenode)) { 
         continue; 
      } else {

#ifdef DEBUG_HAND
	  err_debug("hand_anchors: %d-%s/%s = %s%s(%d)\n", 
		    k, 
		    sent_word_at_index(hand->sent, k), 
		    gram_treename(hand->gram, tree),
		    hand_label(hand, n->treenode), 
		    hand_subscript(hand, n->treenode), 
		    n->treenode);
#endif

         new = nodelist_new(n->treenode);
         if (head) { nodelist_append(head, new); }
         else { head = new; }
      }
   }
  return(head);
}

void
hand_print_feats (hand_t *hand, int i, int tree)
{
  leafinfo_t *n;
  spar_t *s;

  s = hand->lex->lex_mtx;
  err_print("[");
  for (n = spar_get(s,i,tree); n; n = n->next)
    {
      if (n->pruned) { continue; }
      err_print("<%s>", lex_feature_value(n->feat));
    }
  err_print("]");
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
  const grammar_t *gram;
  nodelist_t *new, *head;
  int k;

  head = 0;
  gram = hand->gram;
  for (k = labeltree_start(gram,label); 
       k != NULL_T; 
       k = labeltree_next(gram,k))
    {
      if (lex_is_active(hand->lex, k) && 
	  (gram_tree_type(gram, k) == INIT_TREE)) 
	{
	  new = nodelist_new(hand_rootnode(hand, k));
	  if (head) { nodelist_append(head, new); }
	  else { head = new; }
	}
    }
  return(head);
}

nodelist_t *
hand_aux_rootnodes (hand_t *hand, int label)
{
  const grammar_t *gram;
  nodelist_t *new, *head;
  int k;

  head = 0;
  gram = hand->gram;
  for (k = labeltree_start(gram,label); 
       k != NULL_T; 
       k = labeltree_next(gram,k))
    {
      if (lex_is_active(hand->lex, k) && 
	  (gram_tree_type(gram, k) == AUX_TREE))
	{
	  new = nodelist_new(hand_rootnode(hand, k));
	  if (head) { nodelist_append(head, new); }
	  else { head = new; }
	}
    }
  return(head);
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

