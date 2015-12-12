
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

#ifndef _HAND_H
#define _HAND_H

#include <stdio.h>

#include "gram.h"
#include "corpus.h"
#include "sent.h"
#include "lex.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define SENT_LEN 10000

/* corpus parameters */
#define SPACE    " \t\n"
#define USCORE   "_"
#define LBRACKET "["
#define RBRACKET "]"

  typedef int gnode_t;

  typedef int gtree_t;

  typedef int glabel_t;

  typedef struct hand_s {
    grammar_t *gram;
    corpus_t *corpus;
    FILE *stream;
    sent_t *sent;
    lex_t *lex;
    int in_sent;
  } hand_t;

#ifdef  __cplusplus
}
#endif

/* hand */

hand_t *hand_new (const char *, FILE *);
void hand_delete (hand_t *);
int hand_next_sent (hand_t *);

nodelist_t *hand_anchors (hand_t *, int );
nodelist_t *hand_anchors_for_tree (hand_t *, int , int , int );
void hand_print_feats (hand_t *, int , int );
nodelist_t *hand_init_rootnodes_top (hand_t *, const char *);
nodelist_t *hand_init_rootnodes (hand_t *, int );
nodelist_t *hand_aux_rootnodes (hand_t *, int );
void hand_nodelist_delete (nodelist_t *);

int hand_left_sibling (hand_t *, gnode_t );
int hand_right_sibling (hand_t *, gnode_t );

/* inlines */

static inline unsigned
hand_num_treenodes (const hand_t *h)
{
  return(gram_num_treenodes(h->gram));
}

static inline int
hand_leftchild (const hand_t *h, int node)
{
  return(gram_leftchild(h->gram,node));
}

static inline int
hand_rightflag (const hand_t *h, int node)
{
  return(gram_rightflag(h->gram,node));
}

static inline int
hand_rightnode (const hand_t *h, int node)
{
  return(gram_rightnode(h->gram,node));
}

static inline int
hand_label_index (const hand_t *h, int node)
{
  return(gram_label_index(h->gram,node));
}

static inline int
hand_subscript_index (const hand_t *h, int node)
{
  return(gram_subscript_index(h->gram,node));
}

static inline int
hand_leftnode (const hand_t *h, int node)
{
  return(gram_leftnode(h->gram,node));
}

static inline int
hand_is_unarynode (const hand_t *h, int node)
{
  return(gram_is_unarynode(h->gram,node));
}

static inline int
hand_is_leftnode (const hand_t *h, int node)
{
  return(gram_is_leftnode(h->gram,node));
}

static inline int
hand_is_rightnode (const hand_t *h, int node)
{
  return(gram_is_rightnode(h->gram,node));
}

static inline int
hand_parent (const hand_t *h, int node)
{
  return(gram_parent(h->gram,node));
}

static inline const char *
hand_label (const hand_t *h, int node)
{
  return(gram_label(h->gram,node));
}

static inline const char *
hand_subscript (const hand_t *h, int node)
{
  return(gram_subscript(h->gram,node));
}

static inline int
hand_nodetype (const hand_t *h, int node)
{
  return(gram_nodetype(h->gram,node));
}

static inline int
hand_adjoinable_node (const hand_t *h, int node)
{
  return(gram_adjoinable_node(h->gram,node));
}

static inline int
hand_rightmost_child (const hand_t *hand, int node)
{
  return(gram_rightmost_child(hand->gram,node));
}

static inline int
hand_is_footnode (const hand_t *hand, int node)
{
  return(gram_is_footnode(hand->gram,node));
}

static inline int
hand_is_anchor (const hand_t *hand, int node)
{
  return(gram_is_anchornode(hand->gram,node));
}

static inline int
hand_is_terminal (const hand_t *hand, int node)
{
  return(gram_is_terminalnode(hand->gram,node));
}

static inline int
hand_is_nonterminal (const hand_t *hand, int node)
{
  return((gram_leftchild(hand->gram,node) != NULL_T) ||
	 (gram_is_substnode(hand->gram,node)) ||
	 (gram_is_footnode(hand->gram,node)));
}

static inline int
hand_is_subst (const hand_t *hand, int node)
{
  return(gram_is_substnode(hand->gram, node));
}

static inline gtree_t
hand_tree_from_node (const hand_t *hand, int node)
{
  return(gram_tree_from_node(hand->gram,node));
}

static inline unsigned
hand_sent_length (const hand_t *hand)
{
  return(sent_length(hand->sent));
}

static inline const char *
hand_word_at_index (const hand_t *hand, const int i)
{
  return(sent_word_at_index(hand->sent, i));
}

static inline const char *
hand_treename (const hand_t *hand, gtree_t tree)
{
  return(gram_treename(hand->gram, (int) tree));
}

static inline int
hand_auxiliary_tree (const hand_t *hand, gtree_t tree)
{
  return(gram_auxiliary_tree(hand->gram, (int) tree));
}

static inline int
hand_rootnode (const hand_t *hand, gtree_t tree)
{
  return(gram_rootnode(hand->gram, (int) tree));
}

static inline int
hand_footnode (const hand_t *hand, gtree_t tree)
{
  return(gram_footnode(hand->gram,tree));
}

static inline int
hand_label_from_string (const hand_t *hand, const char *label)
{
  return(gram_label_from_string(hand->gram,label));
}

static inline int
hand_tree_type (const hand_t *hand, gtree_t tree)
{
  return(gram_tree_type(hand->gram, (int) tree));
}

static inline int
hand_leftaux_tree (const hand_t *hand, gnode_t node)
{
  return(gram_leftaux_tree(hand->gram, (int) node));
}

static inline int
hand_rightaux_tree (const hand_t *hand, gnode_t node)
{
  return(gram_rightaux_tree(hand->gram,node));
}

static inline const char *
hand_subscript_seperator (const hand_t *hand, gnode_t node)
{
  return((hand_subscript_index(hand,node) == NULL_SUBSCRIPT) ? "" : "_");
}

/* defines */

/*
 #define hand_num_treenodes(h)         (gram_num_treenodes(h->gram))
 #define hand_leftchild(h,node)        (gram_leftchild(h->gram,node))
 #define hand_rightflag(h,node)        (gram_rightflag(h->gram,node))
 #define hand_rightnode(h,node)        (gram_rightnode(h->gram,node))
 #define hand_label_index(h,node)      (gram_label_index(h->gram,node))
 #define hand_subscript_index(h,node)  (gram_subscript_index(h->gram,node))
 
 #define hand_leftnode(h,node)         (gram_leftnode(h->gram,node))
 				     
 #define hand_parent(h,node)           (gram_parent(h->gram,node))
 #define hand_label(h,node)            (gram_label(h->gram,node))
 #define hand_subscript(h,node)        (gram_subscript(h->gram,node))
 				     
 #define hand_nodetype(h,node)         (gram_nodetype(h->gram,node))
 #define hand_adjoinable_node(h,node)  (gram_adjoinable_node(h->gram,node))
 
 #define hand_rightmost_child(hand,node) (gram_rightmost_child(hand->gram,node))
 #define hand_is_footnode(hand,node)     (gram_is_footnode(hand->gram,node))
 #define hand_is_epsilon(hand,node)      (gram_is_epsilon(hand->gram,gram_label(hand->gram,node)))
 
 #define hand_is_anchor(hand,node)       (gram_is_anchornode(hand->gram,node))
 #define hand_is_terminal(hand,node)     (gram_is_terminalnode(hand->gram,node))
 
 #define hand_is_nonterminal(hand,node)  ((gram_leftchild(hand->gram,node) != NULL_T) || \
					 (gram_is_substnode(hand->gram,node)) || \
					 (gram_is_footnode(hand->gram,node)))
 #define hand_is_subst(hand,node)        (gram_is_substnode(hand->gram, node))
 #define hand_tree_from_node(hand,node)  (gram_tree_from_node(hand->gram,node))
 
 #define hand_sent_length(hand)          (sent_length(hand->sent))
 #define hand_word_at_index(h,i)         (sent_word_at_index(h->sent,i))
 #define hand_treename(hand,tree)        (gram_treename(hand->gram,tree))
 #define hand_auxiliary_tree(hand,tree)  (gram_auxiliary_tree(hand->gram,tree))
 #define hand_rootnode(hand,tree)        (gram_rootnode(hand->gram,tree))
 #define hand_footnode(hand,tree)        (gram_footnode(hand->gram,tree))
 #define hand_label_from_string(h,label) (gram_label_from_string(h->gram,label))
 #define hand_tree_type(hand,tree)       (gram_tree_type(hand->gram,tree))
 #define hand_leftaux_tree(hand,node)    (gram_leftaux_tree(hand->gram,node))
 #define hand_rightaux_tree(hand,node)   (gram_rightaux_tree(hand->gram,node))

 #define hand_subscript_seperator(hand,node) ((hand_subscript_index(hand,node) == NULL_SUBSCRIPT) ? "" : "_")
*/

#endif /* _HAND_H */
