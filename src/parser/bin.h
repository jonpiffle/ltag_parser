
/* bin - set up binary branching elementary trees */

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

#ifndef _BIN_H
#define _BIN_H

#include "gram.h"

#ifdef  __cplusplus
extern "C" {
#endif

  typedef struct bin_s {
    const grammar_t *gram;
    unsigned bin_num_treenodes;      /* gram->num_treenodes + binarizing nodes */
    treenode_t *bin_treenode_tbl;    /* size = num_treenodes */
  } bin_t;

#ifdef  __cplusplus
	   }
#endif

/* bin */

bin_t *bin_new (const grammar_t *);
void bin_delete (bin_t *);

unsigned bin_newnodes (const grammar_t *);
unsigned bin_tree_newnodes (const grammar_t *, int );
unsigned bin_binarynodes (unsigned );

void bin_binarize (bin_t *, const grammar_t *);

void bin_debug_treenodes (const bin_t *);
void bin_pretty_print_trees (const bin_t *);
void bin_pretty_print_tree(const bin_t *, int );

int bin_parent (const bin_t *, int );
int bin_rightmost_child (const bin_t *, int );
const char *bin_label (const bin_t *, int );
const char *bin_subscript (const bin_t *, int );

int bin_nodetype (const bin_t *, int );
int bin_adjoinable_node (const bin_t *, int );

int bin_is_substnode (const bin_t *, int );
int bin_is_anchornode (const bin_t *, int );
int bin_is_footnode (const bin_t *, int );
int bin_is_NAnode (const bin_t *, int );
int bin_tree_from_node (const bin_t *, int );

/* inlines */

static inline unsigned
bin_num_treenodes (const bin_t *bin)
{
  return(bin->bin_num_treenodes);
}

static inline int
bin_leftchild (const bin_t *bin, int node)
{
  return(bin->bin_treenode_tbl[node].leftchild);
}

static inline int
bin_rightflag (const bin_t *bin, int node)
{
  return(bin->bin_treenode_tbl[node].flag);
}

static inline int
bin_rightnode (const bin_t *bin, int node)
{
  return(bin->bin_treenode_tbl[node].right);
}

static inline int
bin_label_index (const bin_t *bin, int node)
{
  return(bin->bin_treenode_tbl[node].label);
}

static inline int
bin_subscript_index (const bin_t *bin, int node)
{
  return(bin->bin_treenode_tbl[node].subscript);
}

/* assumes binary branching */

static inline int
bin_leftnode (const bin_t *bin, int node)
{
  return(bin_leftchild(bin, bin_parent(bin, node)));
}

static inline int
bin_is_unarynode (const bin_t *bin, int node)
{
  return((bin_rightnode(bin, node) == node) ||
	 (bin_leftchild(bin, bin_rightnode(bin, node)) == node));
}

static inline int
bin_is_leftnode (const bin_t *bin, int node)
{
  return(bin_rightflag(bin, node) != PARENT);
}

static inline int
bin_is_rightnode (const bin_t *bin, int node)
{
  return((bin_rightflag(bin, node) == PARENT) &&
	 (bin_leftchild(bin, bin_rightnode(bin, node)) != node));
}

static inline int
bin_is_terminalnode (const bin_t *bin, int node)
{
  return((bin_leftchild(bin, node) == NULL_T) &&
	 (!bin_is_substnode(bin, node)) && 
	 (!bin_is_anchornode(bin, node)) &&
	 (!bin_is_footnode(bin, node)) &&
	 (!gram_is_epsilon(bin->gram, bin_label(bin, node))));
}

/* defines */

/*
 #define bin_num_treenodes(bin)        (bin->bin_num_treenodes)
 #define bin_leftchild(bin,node)       (bin->bin_treenode_tbl[node].leftchild)
 #define bin_rightflag(bin,node)       (bin->bin_treenode_tbl[node].flag)
 #define bin_rightnode(bin,node)       (bin->bin_treenode_tbl[node].right)
 #define bin_label_index(bin,node)     (bin->bin_treenode_tbl[node].label)
 #define bin_subscript_index(bin,node) (bin->bin_treenode_tbl[node].subscript)
*/
			      
/* assumes binary branching */

/*
 #define bin_leftnode(bin,node)        (bin_leftchild(bin, bin_parent(bin,node)))
 #define bin_is_unarynode(bin,node)    ((bin_rightnode(bin,node) == node) || \
				       (bin_leftchild(bin,bin_rightnode(bin,node)) == node))
 #define bin_is_leftnode(bin,node)     (bin_rightflag(bin,node) != PARENT)
 #define bin_is_rightnode(bin,node)    ((bin_rightflag(bin,node) == PARENT) && \
				       (bin_leftchild(bin,bin_rightnode(bin,node)) != node))

 #define bin_is_terminalnode(bin,node) ((bin_leftchild(bin,node) == NULL_T) && \
                                       (!bin_is_substnode(bin,node)) && \
				       (!bin_is_anchornode(bin,node)) && \
				       (!bin_is_footnode(bin,node)) && \
				       (!gram_is_epsilon(bin->gram, bin_label(bin,node))))
*/

#endif /* _BIN_H */
