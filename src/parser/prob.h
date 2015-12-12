
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

#ifndef _PROB_H
#define _PROB_H

#include "hash.h"
#include "gram.h"
#include "postag.h"
#include "prob_counts_db.h"

/* Raw counts for probability model =
 * ----------------------------------
 *
 *   (1) Count(Node, tree, word, tag, NA)
 *       ---------------------------------
 *       Count(Node, tree, word, tag)
 *
 *   COUNT_DIVERSITY TYPE = -1
 *
 *   Count(Node, tree, word, tag, NA) = 
 *   Count(tree, word, tag) - Count(Node, tree, word, tag)
 * 
 *   (1) will be zero for substitution cases
 * 
 *   Pr(tree', tag', word' | Node, tree, word, tag) =
 *   (2)       Pr(tree' | Node, tree, word, tag) *
 *   (3)       Pr(tag'  | tree', Node, tree, word, tag) *
 *   (4)       Pr(word' | tag', tree', Node, tree, word, tag);
 *             Pr(  y   |   x )
 * 
 *   Pr( y | x ) = c(x,y) / c(x)
 *
 *   (2) Count(Node, tree, word, tag, tree') / 
 *   Count(Node, tree, word, tag)
 * 
 *   # c1 = Node, tree, word, tag, tree'
 *   # Backoff: c1.b = Node, tree, tag, tree'
 *   # c2 = Node, tree, word, tag
 *   # Backoff: c2.b = Node, tree, tag
 * 
 *   # (2) == (c1 / c2)  
 *   # Backoff == (c1.b / c2.b)
 * 
 *   COUNT_DIVERSITY TYPE = 4
 * 
 *   (3) Count(tree', Node, tree, word, tag, tag') / 
 *       Count(tree', Node, tree, word, tag)
 * 
 *   # c3 = tree', Node, tree, word, tag, tag'
 *   # Backoff: c3.b = tree', Node, tree, tag, tag'
 *   # c4 = tree', Node, tree, word, tag 
 *   # c4 = c1
 *   # Backoff: c4.b = c1.b
 * 
 *   # (3) == (c3 / c4) == (c3 / c1)
 *   # Backoff == (c3.b / c1.b)
 * 
 *   COUNT_DIVERSITY TYPE = 5
 * 
 *   (4) Count(tag', tree', Node, tree, word, tag, word') / 
 *       Count(tag', tree', Node, tree, word, tag)
 * 
 *   # c5 = tag', tree', Node, tree, word, tag, word'
 *   # Backoff: c5.b = tag', tree', Node, tree, tag, word'
 *   # c6 = tag', tree', Node, tree, word, tag
 *   # c6 = c3
 *   # Backoff: c6.b = tag', tree', Node, tree, tag
 *   # c6.b = c3.b
 * 
 *   # (4) == (c5 / c6) == (c5 / c3)
 *   # Backoff == (c5.b / c3.b)
 * 
 *   COUNT_DIVERSITY TYPE = 6
 **/

#define PROB_ALPHA .00001

#define COUNTS_BUFLEN    8192
#define NODE_BUFLEN      1024
#define PROB_LINE_LEN   10000

#define POSDICT_CACHE_LEN     14
#define UNK_POSDICT_CACHE_LEN  8
#define PROB_CACHE_LEN        14

/*
#define ATTACH_DIVERSITY_FILE "/mnt/linc/home/anoop/work/Cotrain/expt3/data/attach_diversity.dat"
#define COUNT_DIVERSITY_FILE  "/mnt/linc/home/anoop/work/Cotrain/expt3/data/count_diversity.dat"
#define COUNTS_FILE           "/mnt/linc/home/anoop/work/Cotrain/expt3/data/counts.db"
#define TOPLABELS_FILE        "/mnt/linc/home/anoop/work/Cotrain/expt3/data/top_labels.dat"
#define POSDICT_FILE          "/mnt/linc/home/anoop/work/Cotrain/expt3/data/pos_dict.dat"
#define UNK_POSDICT_FILE      "/mnt/linc/home/anoop/work/Cotrain/expt3/data/unknown_pos_dict.dat"

#define NUM_COUNT_DIVERSITY 6

#define DIVERSITY_TYPE_NA -1
#define DIVERSITY_TYPE_A1  4
#define DIVERSITY_TYPE_A2  5
#define DIVERSITY_TYPE_A3  6

#define COUNTS_SEPARATOR "//"
#define COUNTS_NA_STRING "NA"
#define COUNTS_WORD_POSTAG_TOTAL "__word__//__tag__//__TOTAL__"

#define POSTAG_LEN            20
*/

#ifdef  __cplusplus
extern "C" {
#endif

  typedef struct probcache_s {
    char *key;
    double value;
  } probcache_t;

  typedef struct countd_s {
    int count;
    int type;
  } countd_t;

  typedef struct prob_s {
    const grammar_t *gram;

    /* filenames and params read from project */
    char *counts_DB_file;
    char *counts_separator;
    char *counts_NA_string;
    unsigned word_postag_total;
    char *attach_diversity_file;
    char *toplabels_file;
    char *posdict_file;
    char *unknown_posdict_file;
    char *count_diversity_file;
    unsigned num_count_diversity;
    int diversity_type_NA;
    int diversity_type_A1;
    int diversity_type_A2;
    int diversity_type_A3;

    /* data storage */
    int *attach_diversity;       /* size = num_labels */
    countd_t *count_diversity;   /* size = NUM_COUNT_DIVERSITY */
    int *top_labels;             /* size = num_labels */
    unsigned total_top;
    hash_t *postagdict;
    hash_t *unk_postagdict;
    hash_t *cache;
    void *counts;

  } prob_t;

#ifdef  __cplusplus
}
#endif

prob_t *prob_new (const grammar_t *);
void prob_delete (prob_t *);

int prob_get_count (prob_t *, int , ...);
void prob_join_together (prob_t *, char *, int , ...);

double prob_attach (prob_t *, const char *, const char *, const char *, 
		    int , const char *, const char *, const char *);
double prob_NA (prob_t *, int , const char *, const char *, const char *);
double prob_prior (prob_t *, const char *, const char *, const char *);
double prob_init (prob_t *, int );

int prob_conv_to_int (const char *);
double prob_compute_lambda (prob_t *, int , int );
double prob_compute (prob_t *, int , int , int , int , int , int );
int prob_get_count_diversity (prob_t *, int );

void read_attach_diversity (prob_t *);
void read_count_diversity (prob_t *);
void read_top_labels (prob_t *);
int prob_toplabel_count (prob_t *, int );
hash_t *read_postag_dict (const char *, int );

part_of_speech_t *postag_lookup (prob_t *, const char *);
probcache_t *prob_cache_new (const char *, double );
void prob_cache_delete (void *);

#endif /* _PROB_H */
