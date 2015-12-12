
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

#ifndef _GRAM_H
#define _GRAM_H

#include "pref.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define PRJ_LINELEN    2048
#define TREENAME_LEN   1024
#define FAMILYNAME_LEN 1024
#define FEAT_LINELEN   16384
#define SYNDEFAULT_LEN 524288

#define NULL_T         -1
#define NULL_SUBSCRIPT -1
#define NULL_LABEL     -1

/* see $LEMHOME/data/README.data for the formats of the tree files */
#define TREENODES_FORMAT "%d %d %d %d %d\n"
#define TREES_FORMAT     "%d %d %d %d %d"
#define LIST_FORMAT      "%d"
#define FAMILY_FORMAT    "%s %d"
#define EPSILON_FORMAT   "%s %s\n"
#define TOPLABEL_FORMAT  "%s %s\n"
#define NUMBER           "%d\n"
#define FILENAME         "%s\n"

/* if the FLAG field in treenodes.dat is 1 then RIGHT == PARENT */
#define PARENT    1

#define INIT_TREE 1
#define AUX_TREE  2

#define IS_INTERNAL -1
#define IS_FOOT     0
#define IS_ANCHOR   1
#define IS_SUBST    2
#define IS_TERMINAL 3
#define IS_EPS      4

  typedef struct eps_s {
    char *empty_elt;
    int  label;
  } eps_t;

  typedef struct toplabel_s {
    int  toplabel;
    char *startfeat;
  } toplabel_t;

  typedef struct treenode_s {
    int leftchild;
    int right;
    int flag;
    int label;
    int subscript;
  } treenode_t;

  typedef struct grammar_s {

    /* data from project file */
    unsigned num_trees;
    unsigned num_families;
    unsigned num_treenodes;
    unsigned num_labels;
    unsigned num_subscripts;
    unsigned width_labels;
    unsigned width_subscripts;
    unsigned num_eps;
    unsigned num_toplabels;

    /* filenames from project file */
    char *elementary_trees;
    char *tree_names;
    char *tree_nodes;
    char *tree_features;
    char *tree_families;
    char *tree_labels;
    char *tree_subscripts;
    char *epsilons;
    char *toplabels;
    char *modelfile;
    char *default_file;

    /* syn default entry */
    unsigned num_syndefault;
    char **syndefault;

    /* grammar data */

    /* size = num_treenodes */
    treenode_t *treenode_tbl;
    int        *inverted_subst_tbl;
    int        *inverted_foot_tbl;
    int        *inverted_anchor_tbl;
    int        *inverted_NA_tbl;

    /* size = num_labels */
    char **label_tbl;

    /* size = num_subscripts */
    char **subscript_tbl;

    /* size = num_trees */
    int   *tree_tbl;
    int   *footnode_tbl;
    int   *num_subst_tbl;
    int   *num_anchor_tbl;
    int   *num_NA_tbl;
    int  **subst_list_tbl;          /* size of subst_list_tbl[i] = num_subst_tbl[i] */
    int  **anchor_list_tbl;         /* size of anchor_list_tbl[i] = num_anchor_tbl[i] */
    int  **NA_list_tbl;             /* size of NA_list_tbl[i] = num_NA_tbl[i] */
    char **treename_tbl;
    char **treefeat_tbl;

    /* size = num_families */
    char **family_tbl;
    int   *family_size;
    int  **family_tree;

    /* word to tree/family database */
    char *dbfile;
    void *dbp;

    /* values for epsilon */
    eps_t *eps_tbl;

    /* table of toplabels */
    toplabel_t *toplabel_tbl;

    /* additional treeinfo */
    int *inverted_tree_tbl;         /* size = num_treenodes */
    int *num_terminal_tbl;          /* size = num_trees */
    int *num_adjoinable_tbl;        /* size = num_trees */
    int **terminal_list_tbl;        /* size[i] = num_terminal_tbl[i] */
    int **adjoinable_list_tbl;      /* size[i] = num_adjoinable_tbl[i] */

    int *labeltree_tbl;             /* size = num_labels */
    int *labeltree_list_tbl;        /* size = num_trees */

  } grammar_t;

  typedef struct nodelist_s {
    int treenode;
    struct nodelist_s *next;
  } nodelist_t;

#ifdef	__cplusplus
}
#endif

/* gram */

extern grammar_t *gram_new    (const char *);
extern void       gram_delete (grammar_t *);

void project_debug       (grammar_t *);
int  project_getindex    (char *);
void project_linesplit   (char *, char **, char **);

int gram_toplabel_index  (grammar_t *, int );

/* gram_slurp */

void read_labels         (grammar_t *);
void delete_labels       (grammar_t *);
void read_subscripts     (grammar_t *);
void delete_subscripts   (grammar_t *);
void read_treenodes      (grammar_t *);
void delete_treenodes    (grammar_t *);
void read_trees          (grammar_t *);
void delete_trees        (grammar_t *);
void read_treenames      (grammar_t *);
void delete_treenames    (grammar_t *);
void read_treefeatures   (grammar_t *);
void delete_treefeatures (grammar_t *);
void read_families       (grammar_t *);
void delete_families     (grammar_t *);
void read_epsilons       (grammar_t *);
void delete_epsilons     (grammar_t *);
void read_toplabels      (grammar_t *);
void delete_toplabels    (grammar_t *);
void read_syndefault     (grammar_t *);
void delete_syndefault   (grammar_t *);

/* gram_trees */

extern int gram_treeindex        (const grammar_t *, const char *);
extern int gram_familyindex      (const grammar_t *, const char *);
extern int gram_index_from_label (const grammar_t *, const char *);

extern const char *gram_subscript (const grammar_t *, int );

extern int gram_treenode_cmp (const grammar_t *, int , int , int );
int gram_treenode_cmp_helper (const grammar_t *, int , int , int );

extern int gram_is_epsilon         (const grammar_t *, const char *);
extern int gram_aux_tree_has_yield (const grammar_t *, int );
extern int gram_leftaux_tree       (const grammar_t *, int );
extern int gram_rightaux_tree      (const grammar_t *, int );
extern int gram_parent             (const grammar_t *, int );
extern int gram_rightmost_child    (const grammar_t *, int );
extern int gram_leftnode           (const grammar_t *, int );

/* gram_nodelist */

nodelist_t *nodelist_new (int );
nodelist_t *nodelist_append (nodelist_t *, nodelist_t *);
void nodelist_delete (nodelist_t *);
unsigned nodelist_length (nodelist_t *);
void nodelist_debug (const grammar_t *, nodelist_t *);

/* gram_treemap */

extern void *gram_tree_map (const grammar_t *, 
			    void *(*) (const grammar_t *, int ), 
			    void *(*) (void *, void *), 
			    int );
extern void *gram_print_node (const grammar_t *, int );
extern void *gram_null_append (void *, void *);
extern void *gram_terminal (const grammar_t *, int );
extern void *gram_terminal_or_foot (const grammar_t *, int );
extern void *nodelist_new_wrap (const grammar_t *, int );
extern void *nodelist_app_wrap (void *, void *);

/* gram_treeinfo */

void gram_treeinfo (grammar_t *);

int gram_nodetype (const grammar_t *, int );
int gram_adjoinable_node (const grammar_t *, int );

void *gram_adjoinable (const grammar_t *, int );
void gram_setup_treeinfo (grammar_t *, int );
void gram_delete_treeinfo (grammar_t *);
void gram_debug_treeinfo (const grammar_t *);

int gram_getindex (int , int , int *);
int gram_anchor_getindex (const grammar_t *, int , int );
int gram_terminal_getindex (const grammar_t *, int , int );
int gram_subst_getindex (const grammar_t *, int , int );
int gram_adjoinable_getindex (const grammar_t *, int , int );

unsigned gram_numchildren (const grammar_t *, int );
int gram_label_from_string (const grammar_t *, const char *);

/* inlines */

static inline unsigned
gram_num_trees (const grammar_t *gram)
{
  return(gram->num_trees);
}

static inline unsigned
gram_num_treenodes (const grammar_t *gram)
{
  return(gram->num_treenodes);
}

static inline unsigned
gram_num_labels (const grammar_t *gram)
{
  return(gram->num_labels);
}

static inline int
gram_familysize (const grammar_t *gram, int family)
{
  return(gram->family_size[family]);
}

static inline int
gram_family_tree (const grammar_t *gram, int family, int tree)
{
  return(gram->family_tree[family][tree]);
}

static inline int
gram_leftchild (const grammar_t *gram, int treenode)
{
  return(gram->treenode_tbl[treenode].leftchild);
}

static inline int
gram_rightnode (const grammar_t *gram, int treenode)
{
  return(gram->treenode_tbl[treenode].right);
}

static inline int
gram_rightflag (const grammar_t *gram, int treenode)
{
  return(gram->treenode_tbl[treenode].flag);
}

static inline char *
gram_label (const grammar_t *gram, int treenode)
{
  return(gram->label_tbl[gram->treenode_tbl[treenode].label]);
}

static inline char *
gram_label_from_index (const grammar_t *gram, int lindex)
{
  return(gram->label_tbl[lindex]);
}

static inline int
gram_label_index (const grammar_t *gram, int treenode)
{
  return(gram->treenode_tbl[treenode].label);
}

static inline int
gram_subscript_index (const grammar_t *gram, int treenode)
{
  return(gram->treenode_tbl[treenode].subscript);
}

static inline int
gram_valid_treenode (const grammar_t *gram, int node)
{
  return((node > NULL_T) && (node < gram_num_treenodes(gram)));
}

static inline int
gram_tree_from_anchor (const grammar_t *gram, int treenode)
{
  return(gram->inverted_anchor_tbl[treenode]);
}

static inline const char *
gram_treename (const grammar_t *gram, int tree)
{
  return((tree == NULL_T) ? "[UNK]" : (gram->treename_tbl[tree]));
}

static inline int
gram_tree_from_node (const grammar_t *gram, int treenode)
{
  return(gram->inverted_tree_tbl[treenode]);
}

static inline int
gram_anchor_from_index (const grammar_t *gram, int tree, int windex)
{
  return(gram->anchor_list_tbl[tree][windex]);
}

static inline int
gram_subst_from_index (const grammar_t *gram, int tree, int windex)
{
  return(gram->subst_list_tbl[tree][windex]);
}

static inline int
gram_NA_from_index (const grammar_t *gram, int tree, int windex)
{
  return(gram->NA_list_tbl[tree][windex]);
}

static inline int
gram_terminal_from_index (const grammar_t *gram, int tree, int windex)
{
  return(gram->terminal_list_tbl[tree][windex]);
}

static inline int
gram_adjoinable_from_index (const grammar_t *gram, int tree, int windex)
{
  return(gram->adjoinable_list_tbl[tree][windex]);
}

static inline int
gram_num_anchors (const grammar_t *gram, int tree)
{
  return(gram->num_anchor_tbl[tree]);
}

static inline int
gram_num_subst (const grammar_t *gram, int tree)
{
  return(gram->num_subst_tbl[tree]);
}

static inline int
gram_num_NA (const grammar_t *gram, int tree)
{
  return(gram->num_NA_tbl[tree]);
}

static inline int
gram_num_terminals (const grammar_t *gram, int tree)
{
  return(gram->num_terminal_tbl[tree]);
}

static inline int
gram_num_adjoinable (const grammar_t *gram, int tree)
{
  return(gram->num_adjoinable_tbl[tree]);
}

static inline int
gram_rootnode (const grammar_t *gram, int tree)
{
  return(gram->tree_tbl[tree]);
}

static inline int
gram_footnode (const grammar_t *gram, int tree)
{
  return(gram->footnode_tbl[tree]);
}

static inline int
gram_is_substnode (const grammar_t *gram, int treenode)
{
  return((gram->inverted_subst_tbl[treenode] != NULL_T) ? 1 : 0);
}

static inline int
gram_is_anchornode (const grammar_t *gram, int treenode)
{
  return((gram->inverted_anchor_tbl[treenode] != NULL_T) ? 1 : 0);
}

static inline int
gram_is_footnode (const grammar_t *gram, int treenode)
{
  return((gram->inverted_foot_tbl[treenode] != NULL_T) ? 1 : 0);
}

static inline int
gram_is_NAnode (const grammar_t *gram, int treenode)
{
  return((gram->inverted_NA_tbl[treenode] != NULL_T) ? 1 : 0);
}

static inline int
gram_is_terminalnode (const grammar_t *gram, int treenode)
{
  return((gram_leftchild(gram, treenode) == NULL_T) &&
	 (!gram_is_substnode(gram, treenode)) &&
	 (!gram_is_anchornode(gram, treenode)) &&
	 (!gram_is_footnode(gram, treenode)) &&
	 (!gram_is_epsilon(gram, gram_label(gram, treenode))));
}

static inline int
gram_initial_tree (const grammar_t *gram, int tree)
{
  return((gram->footnode_tbl[tree] == -1) ? 1 : 0);
}

static inline int
gram_auxiliary_tree (const grammar_t *gram, int tree)
{
  return((gram->footnode_tbl[tree] == -1) ? 0 : -1);
}

static inline int
gram_tree_has_one_anchor (const grammar_t *gram, int tree)
{
  return(gram->num_anchor_tbl[tree] == 1);
}

static inline int
gram_tree_type (const grammar_t *gram, int tree)
{
  return((gram->footnode_tbl[tree] == -1) ? INIT_TREE : AUX_TREE);
}


static inline char *
gram_empty_elt (const grammar_t *gram, int windex)
{
  return(gram->eps_tbl[windex].empty_elt);
}

static inline int
gram_epsilon_label (const grammar_t *gram, int windex)
{
  return(gram->eps_tbl[windex].label);
}

static inline unsigned
gram_num_eps (const grammar_t *gram)
{
  return(gram->num_eps);
}

static inline int
labeltree_start (const grammar_t *gram, int label)
{
  return((gram->labeltree_tbl[label] == NULL_LABEL) ?
	 NULL_T : gram->labeltree_tbl[label]);
}

static inline int
labeltree_next (const grammar_t *gram, int k)
{
  return(gram->labeltree_list_tbl[k]);
}

/* defines */

/*
 #define gram_num_trees(gram)                  (gram->num_trees)
 #define gram_num_treenodes(gram)              (gram->num_treenodes)
 #define gram_num_labels(gram)                 (gram->num_labels)

 #define gram_familysize(gram,family)          (gram->family_size[family])
 #define gram_family_tree(gram,family,tree)    (gram->family_tree[family][tree])

 #define gram_leftchild(gram,treenode)         (gram->treenode_tbl[treenode].leftchild)
 #define gram_rightnode(gram,treenode)         (gram->treenode_tbl[treenode].right)
 #define gram_rightflag(gram,treenode)         (gram->treenode_tbl[treenode].flag)

 #define gram_label(gram,treenode)             (gram->label_tbl[gram->treenode_tbl[treenode].label])
 #define gram_label_from_index(gram,windex)     (gram->label_tbl[windex])
 #define gram_label_index(gram,treenode)       (gram->treenode_tbl[treenode].label)
 #define gram_subscript_index(gram,treenode)   (gram->treenode_tbl[treenode].subscript)

 #define gram_valid_treenode(gram,node)        ((node > NULL_T) && (node < gram_num_treenodes(gram)))
 #define gram_tree_from_anchor(gram,treenode)  (gram->inverted_anchor_tbl[treenode])

 #define gram_treename(gram,tree)              ((tree == NULL_T) ? "[UNK]" : (gram->treename_tbl[tree]))
 #define gram_tree_from_node(gram,treenode)    (gram->inverted_tree_tbl[treenode])

 #define gram_anchor_from_index(gram,tree,windex)     (gram->anchor_list_tbl[tree][windex])
 #define gram_subst_from_index(gram,tree,windex)      (gram->subst_list_tbl[tree][windex])
 #define gram_NA_from_index(gram,tree,windex)         (gram->NA_list_tbl[tree][windex])
 #define gram_terminal_from_index(gram,tree,windex)   (gram->terminal_list_tbl[tree][windex])
 #define gram_adjoinable_from_index(gram,tree,windex) (gram->adjoinable_list_tbl[tree][windex])

 #define gram_num_anchors(gram,tree)           (gram->num_anchor_tbl[tree])
 #define gram_num_subst(gram,tree)             (gram->num_subst_tbl[tree])
 #define gram_num_NA(gram,tree)                (gram->num_NA_tbl[tree])
 #define gram_num_terminals(gram,tree)         (gram->num_terminal_tbl[tree])
 #define gram_num_adjoinable(gram,tree)        (gram->num_adjoinable_tbl[tree])

 #define gram_rootnode(gram,tree)              (gram->tree_tbl[tree])
 #define gram_footnode(gram,tree)              (gram->footnode_tbl[tree])

 #define gram_is_substnode(gram,treenode)      ((gram->inverted_subst_tbl[treenode] != NULL_T) ? 1 : 0)
 #define gram_is_anchornode(gram,treenode)     ((gram->inverted_anchor_tbl[treenode] != NULL_T) ? 1 : 0)
 #define gram_is_footnode(gram,treenode)       ((gram->inverted_foot_tbl[treenode] != NULL_T) ? 1 : 0)
 #define gram_is_NAnode(gram,treenode)         ((gram->inverted_NA_tbl[treenode] != NULL_T) ? 1 : 0)
 #define gram_is_terminalnode(gram,treenode)   ((gram_leftchild(gram, treenode) == NULL_T) && \
					       (!gram_is_substnode(gram, treenode)) && \
					       (!gram_is_anchornode(gram, treenode)) && \
					       (!gram_is_footnode(gram, treenode)) && \
					       (!gram_is_epsilon(gram, gram_label(gram, treenode))))

 #define gram_initial_tree(gram,tree)          ((gram->footnode_tbl[tree] == -1) ? 1 : 0)
 #define gram_auxiliary_tree(gram,tree)        ((gram->footnode_tbl[tree] == -1) ? 0 : -1)
 #define gram_tree_has_one_anchor(gram,tree)   (gram->num_anchor_tbl[tree] == 1)
 #define gram_tree_type(gram,tree)             ((gram->footnode_tbl[tree] == -1) ? INIT_TREE : AUX_TREE)

 #define gram_empty_elt(gram,windex)            (gram->eps_tbl[windex].empty_elt)
 #define gram_epsilon_label(gram,windex)        (gram->eps_tbl[windex].label)
 #define gram_num_eps(gram)                    (gram->num_eps)


 #define labeltree_start(gram,label)           ((gram->labeltree_tbl[label] == NULL_LABEL) ? \
					       NULL_T : gram->labeltree_tbl[label])
 #define labeltree_next(gram,k)                (gram->labeltree_list_tbl[k])
*/

#endif /* _GRAM_H */
