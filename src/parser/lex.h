
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

#ifndef _LEX_H
#define _LEX_H

#include "pref.h"
#include "spar.h"
#include "gram.h"
#include "sent.h"
#include "syn_entry.h"

#define SENT_LEN 10000

#ifdef  __cplusplus
extern "C" {
#endif

  typedef struct epstree_s {
    char *key;
    int   treenode;
    char *feat;
  } epstree_t;

  typedef struct leafinfo_s {
    unsigned pruned;
    int      treenode;
    char    *feat;
    char    *postag;
    struct leafinfo_s *next;
  } leafinfo_t;

  typedef struct lex_s {
    const grammar_t *gram;
    const sent_t    *sent;
    spar_t          *lex_mtx;           /* size = [ sent->length x gram->num_trees ] */
    unsigned         max_eps;
    unsigned         num_eps;
    epstree_t       *eps_tbl;           /* size = max_eps */
    int             *active_tbl;        /* size = gram->num_trees */
  } lex_t;

#ifdef  __cplusplus
	   }
#endif

/* lex */
extern lex_t   *lex_new (const sent_t *, const grammar_t *);
extern void     lex_delete (lex_t *);
extern void     lex_debug (lex_t *);

extern lex_t *lex_nbest_new (const sent_t *, const grammar_t *);
extern int lex_nbest (lex_t *, int , char *, char *, char *, char *);

void  lex_lexicalize (lex_t *, syn_entry_t *);
void  lex_get_terminals (lex_t *, syn_entrylist_t *, int );
int   lex_update (lex_t *, syn_entrylist_t *, int , int , const char *);
char *lex_match (const grammar_t *, syn_list_t *, syn_list_t *, int );
char *lex_merge (const char *, const char *);

int lex_is_eps_anchor (lex_t *, int );
char *lex_eps_feature_value (lex_t *, int );

char *lex_feature_new (syn_list_t *);
const char *lex_feature_value (char *);
void lex_feature_delete (char *);

char *lex_postag_new (const char *p);
void lex_postag_delete (char *p);

leafinfo_t *leafinfo_new (int , char *, char *, unsigned );
void leafinfo_delete (leafinfo_t *);

int lex_multiple_anchor_check_key (char **, unsigned );
int lex_multiple_anchor_count (char *, int , char **, unsigned );
void lex_update_multiple_anchor (lex_t *, syn_entrylist_t *, char **, unsigned , int );

void lex_mtx_debug (lex_t *);
void lex_eps_debug (lex_t *);

/* lex_prune */
void lex_prune (lex_t *);
void lex_prune_tree (lex_t *, int );
void lex_prune_check (lex_t *, int , leafinfo_t *, leafinfo_t *, int , int );
int  lex_info_all_pruned (leafinfo_t *);

/* lex_active */

extern int *lex_active (lex_t *);
void lex_active_debug (lex_t *);

/* inlines */

static inline int
lex_is_active (const lex_t *l, int tree)
{
  return(l->active_tbl[tree]);
}

static inline const grammar_t *
lex_gram (const lex_t *l)
{
  return(l->gram);
}

static inline const sent_t *
lex_sent (const lex_t *l)
{
  return(l->sent);
}

static inline spar_t *
lex_mtx (const lex_t *l)
{
  return(l->lex_mtx);
}

/* defines */

/*
 #define lex_is_active(l,tree)     (l->active_tbl[tree])
 #define lex_gram(l)               (l->gram)
 #define lex_sent(l)               (l->sent)
 #define lex_mtx(l)                (l->lex_mtx)
*/

#endif /* _LEX_H */
