
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

#include "bin.h"
#include "prob.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define SENT_LEN 10000
#define SUBSCRIPT_SEPERATOR "_"

/* corpus parameters */
#define SPACE    " \t\n"
#define USCORE   "_"
#define LBRACKET "["
#define RBRACKET "]"

/* defines added for headcorner */
#define UNK_HEAD        -1
#define NULL_SPAN       -1

  typedef int gnode_t;

  typedef int gtree_t;

  typedef int glabel_t;

  typedef leafinfo_t anchor_t;

  typedef struct hand_s {
    grammar_t *gram;
    corpus_t *corpus;
    FILE *stream;
    sent_t *sent;
    lex_t *lex;
    int in_sent;

    nodelist_t **init_rootnode_tbl;  /* size = num_labels */
    nodelist_t **aux_rootnode_tbl;   /* size = num_labels */

    int *leftaux_tbl;  /* size = num_treenodes */
    int *rightaux_tbl; /* size = num_treenodes */

    bin_t *bin;

    int *headcorner_tbl;             /* size = num_treenodes */

    prob_t *prob;

    /* buffers for processing the input */
    char *sentbuf;
    char *nbest_linebuf;
    char *wordbuf;
    char *postagbuf;
    char *anchorbuf;
    char *treenamebuf;

  } hand_t;

#ifdef  __cplusplus
}
#endif

/* hparse_hand */

hand_t *hand_new (const char *, FILE *);
void hand_delete (hand_t *);
int hand_next_sent (hand_t *);

anchor_t *hand_anchors_for_tree (hand_t *, int , int , int , anchor_t *);
void hand_print_feats (hand_t *, int , int );
nodelist_t *hand_init_rootnodes_top (hand_t *, const char *);
nodelist_t *hand_init_rootnodes (hand_t *, int );
nodelist_t *hand_aux_rootnodes (hand_t *, int );
void hand_nodelist_delete (nodelist_t *);

int hand_left_sibling (hand_t *, gnode_t );
int hand_right_sibling (hand_t *, gnode_t );

int hand_find_headcorner (hand_t *, int );
int hand_cmp_headcorner (int , int );

/* inlines */

static inline unsigned
hand_num_toplabels (const hand_t *h)
{
  return((h->gram->num_toplabels));
}

static inline int
hand_toplabel (const hand_t *h, unsigned i)
{
  return((h->gram->toplabel_tbl[i].toplabel));
}

static inline int
hand_is_toplabel (const hand_t *h, int label)
{
  return(((gram_toplabel_index(h->gram, label) == -1) ? 0 : 1));
}

static inline const char *
hand_toplabel_feat (const hand_t *h, int label)
{
  return((h->gram->toplabel_tbl[gram_toplabel_index(h->gram, label)].startfeat));
}

/* assumes binary branching (see bin.c) */

static inline unsigned
hand_num_treenodes (const hand_t *h)
{
  return(bin_num_treenodes(h->bin));
}

static inline int
hand_leftchild (const hand_t *h, int node)
{
  return(bin_leftchild(h->bin,node));
}

static inline int
hand_rightflag (const hand_t *h, int node)
{
  return(bin_rightflag(h->bin,node));
}

static inline int
hand_rightnode (const hand_t *h, int node)
{
  return(bin_rightnode(h->bin,node));
}

static inline int
hand_label_index (const hand_t *h, int node)
{
  return(bin_label_index(h->bin,node));
}

static inline int
hand_subscript_index (const hand_t *h, int node)
{
  return(bin_subscript_index(h->bin,node));
}

static inline int
hand_leftnode (const hand_t *h, int node)
{
  return(bin_leftnode(h->bin,node));
}

static inline int
hand_is_unarynode (const hand_t *h, int node)
{
  return(bin_is_unarynode(h->bin,node));
}

static inline int
hand_is_leftnode (const hand_t *h, int node)
{
  return(bin_is_leftnode(h->bin,node));
}

static inline int
hand_is_rightnode (const hand_t *h, int node)
{
  return(bin_is_rightnode(h->bin,node));
}

static inline int
hand_parent (const hand_t *h, int node)
{
  return(bin_parent(h->bin,node));
}

static inline const char *
hand_label (const hand_t *h, int node)
{
  return(bin_label(h->bin,node));
}

static inline const char *
hand_subscript (const hand_t *h, int node)
{
  return(bin_subscript(h->bin,node));
}

static inline int
hand_nodetype (const hand_t *h, int node)
{
  return(bin_nodetype(h->bin,node));
}

static inline int
hand_adjoinable_node (const hand_t *h, int node)
{
  return(bin_adjoinable_node(h->bin,node));
}

static inline int
hand_rightmost_child (const hand_t *hand, int node)
{
  return(bin_rightmost_child(hand->bin,node));
}

static inline int
hand_is_footnode (const hand_t *hand, int node)
{
  return(bin_is_footnode(hand->bin,node));
}

static inline int
hand_is_anchor (const hand_t *hand, int node)
{
  return(bin_is_anchornode(hand->bin,node));
}

static inline int
hand_is_terminal (const hand_t *hand, int node)
{
  return(bin_is_terminalnode(hand->bin,node));
}

static inline int
hand_is_nonterminal (const hand_t *hand, int node)
{
  return((bin_leftchild(hand->bin,node) != NULL_T) ||
	 (bin_is_substnode(hand->bin,node)) ||
	 (bin_is_footnode(hand->bin,node)));
}

static inline int
hand_is_subst (const hand_t *hand, int node)
{
  return(bin_is_substnode(hand->bin, node));
}

static inline gtree_t
hand_tree_from_node (const hand_t *hand, int node)
{
  return(bin_tree_from_node(hand->bin,node));
}

/* independent of bin.c */

static inline unsigned
hand_num_labels (const hand_t *hand)
{
  return(gram_num_labels(hand->gram));
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

/*
static inline int
hand_leftaux_tree (const hand_t *hand, gnode_t node)
{
  return(gram_leftaux_tree(hand->gram, (int) node));
}
*/

static inline int
hand_leftaux_tree (const hand_t *hand, gnode_t node)
{
  return(hand->leftaux_tbl[node]);
}

/*
static inline int
hand_rightaux_tree (const hand_t *hand, gnode_t node)
{
  return(gram_rightaux_tree(hand->gram,node));
}
*/

static inline int
hand_rightaux_tree (const hand_t *hand, gnode_t node)
{
  return(hand->rightaux_tbl[node]);
}

static inline const char *
hand_subscript_seperator (const hand_t *hand, gnode_t node)
{
  return((hand_subscript_index(hand,node) == NULL_SUBSCRIPT) ? "" : SUBSCRIPT_SEPERATOR);
}

static inline unsigned
hand_num_eps (const hand_t *hand)
{
  return(hand->lex->num_eps);
}

static inline char *
hand_eps_tbl_key (const hand_t *hand, int windex)
{
  return(hand->lex->eps_tbl[windex].key);
}

static inline gnode_t
hand_eps_tbl_treenode (const hand_t *hand, int windex)
{
  return(hand->lex->eps_tbl[windex].treenode);
}

static inline char *
hand_eps_tbl_feat (const hand_t *hand, int windex)
{
  return(hand->lex->eps_tbl[windex].feat);
}

static inline int
hand_is_eps_anchor (const hand_t *hand, int treenode)
{
  return(lex_is_eps_anchor(hand->lex, treenode));
}

/* assumes headcorner computation (see hparse_hand.c) */

static inline gnode_t
hand_headcorner (const hand_t *hand, gnode_t node)
{
  return((gnode_t) hand->headcorner_tbl[node]);
}

/* defines */

/*
 #define hand_num_treenodes(h)        (bin_num_treenodes(h->bin))
 #define hand_leftchild(h,node)       (bin_leftchild(h->bin,node))
 #define hand_rightflag(h,node)       (bin_rightflag(h->bin,node))
 #define hand_rightnode(h,node)       (bin_rightnode(h->bin,node))
 #define hand_label_index(h,node)     (bin_label_index(h->bin,node))
 #define hand_subscript_index(h,node) (bin_subscript_index(h->bin,node))

 #define hand_leftnode(h,node)        (bin_leftnode(h->bin,node))
 #define hand_is_unarynode(h,node)    (bin_is_unarynode(h->bin,node))
 #define hand_is_leftnode(h,node)     (bin_is_leftnode(h->bin,node))
 #define hand_is_rightnode(h,node)    (bin_is_rightnode(h->bin,node))
				     
 #define hand_parent(h,node)          (bin_parent(h->bin,node))
 #define hand_label(h,node)           (bin_label(h->bin,node))
 #define hand_subscript(h,node)       (bin_subscript(h->bin,node))

 #define hand_nodetype(h,node)        (bin_nodetype(h->bin,node))
 #define hand_adjoinable_node(h,node) (bin_adjoinable_node(h->bin,node))

 #define hand_rightmost_child(hand,node) (bin_rightmost_child(hand->bin,node))
 #define hand_is_footnode(hand,node)     (bin_is_footnode(hand->bin,node))
 #define hand_is_epsilon(hand,node)      (gram_is_epsilon(hand->gram,bin_label(hand->bin,node)))

 #define hand_is_anchor(hand,node)       (bin_is_anchornode(hand->bin,node))
 #define hand_is_terminal(hand,node)     (bin_is_terminalnode(hand->bin,node))

 #define hand_is_nonterminal(hand,node)  ((bin_leftchild(hand->bin,node) != NULL_T) || \
					 (bin_is_substnode(hand->bin,node)) || \
					 (bin_is_footnode(hand->bin,node)))
 #define hand_is_subst(hand,node)        (bin_is_substnode(hand->bin, node))
 #define hand_tree_from_node(hand,node)  (bin_tree_from_node(hand->bin,node))
*/

/* independent of bin.c */

/*
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

 #define hand_num_eps(hand)                (hand->lex->num_eps)
 #define hand_eps_tbl_key(hand,windex)      (hand->lex->eps_tbl[windex].key)
 #define hand_eps_tbl_treenode(hand,windex) (hand->lex->eps_tbl[windex].treenode)
 #define hand_eps_tbl_feat(hand,windex)     (hand->lex->eps_tbl[windex].feat)

*/

/* assumes headcorner computation (see hparse_hand.c) */
/*
 #define hand_headcorner(hand,node)      (hand->headcorner_tbl[node])
*/

#endif /* _HAND_H */
