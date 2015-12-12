
/* prob - manage probability models for the parser */

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

static char rcsid[] = "$Id: prob.c,v 1.18 2000/10/21 17:12:03 anoop Exp $";

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "myassert.h"
#include "prob.h"
#include "prob_project.h"
#include "err.h"
#include "mem.h"

#define default_assign(s)  (s = (s) ? s : "-")

static int
prob_hash_sz (int len)
{
  if (len < 16) { return(pow(2.0,(len+1))); }
  else { return(262144); }
}

static int 
prob_project_getindex (char *tag)
{
  int sz = prob_tags[0].index;
  int i;
  for (i = 1; i < sz; i++)
    {
      if (strcmp(tag, prob_tags[i].tag) == 0) {
	return(prob_tags[i].index);
      }
    }
  return(0); /* syntax error */
}

static void
prob_project_linesplit (char *line, char **attr, char **value)
{
  const char *seperator = prob_tags[0].tag;
  *attr = strtok(line, seperator);
  *value = strtok(0, seperator);
  if ((*attr == NULL) || (*value == NULL)) {
    err_abort("read_prob", "syntax error. perhaps you have spaces in the file\n");
  }
}

prob_t *
prob_new (const grammar_t *gram)
{
  FILE *project_f;
  int tag_index, linenum, fstatus = 0;
  char line[PROB_LINE_LEN];
  char *attr, *value;
  prob_t *new;
  int verbose = 0;

  if (gram->modelfile == 0) {
    err_abort("prob_new", "invalid model file in grammar\n");
  }

  if ((project_f = fopen(gram->modelfile, "r")) == NULL) {
    err_abort("prob_new", "could not open model file %s\n", gram->modelfile);
  }
  new = (prob_t *) mem_alloc(sizeof(prob_t));
  new->gram = gram;

  if (!err_be_quiet()) { 
    verbose = 1;
  }

  linenum = 0;
  while ((fstatus = fscanf(project_f, "%s\n", line)))
    {
      if (fstatus == EOF) break;
      linenum++;
      prob_project_linesplit(line, &attr, &value);
      if ((tag_index = prob_project_getindex(attr)) == 0) {
	err_warning("prob_new", "syntax error in %s line %d\n", gram->modelfile, linenum);
	continue;
      }
      if (verbose) err_debug("%s=", prob_tags[tag_index].tag);
      switch(tag_index) {
      case COUNTS_DB:
	new->counts_DB_file = mem_strdup(value);
	if (verbose) err_debug("%s\n", new->counts_DB_file);
	break;
      case COUNTS_SEPARATOR:
	new->counts_separator = mem_strdup(value);
	if (verbose) err_debug("%s\n", new->counts_separator);
	break;
      case COUNTS_NA_STRING:
	new->counts_NA_string = mem_strdup(value);
	if (verbose) err_debug("%s\n", new->counts_NA_string);
	break;
      case COUNTS_WORD_POSTAG_TOTAL:
	new->word_postag_total = atoi(value);
	if (verbose) err_debug("%d\n", new->word_postag_total);
	break;
      case ATTACH_DIVERSITY_FILE:
	new->attach_diversity_file = mem_strdup(value);
	if (verbose) err_debug("%s\n", new->attach_diversity_file);
	break;
      case TOPLABELS_FILE:
	new->toplabels_file = mem_strdup(value);
	if (verbose) err_debug("%s\n", new->toplabels_file);
	break;
      case POSDICT_FILE:
	new->posdict_file = mem_strdup(value);
	if (verbose) err_debug("%s\n", new->posdict_file);
	break;
      case UNK_POSDICT_FILE:
	new->unknown_posdict_file = mem_strdup(value);
	if (verbose) err_debug("%s\n", new->unknown_posdict_file);
	break;
      case COUNT_DIVERSITY_FILE:
	new->count_diversity_file = mem_strdup(value);
	if (verbose) err_debug("%s\n", new->count_diversity_file);
	break;
      case NUM_COUNT_DIVERSITY:
	new->num_count_diversity = atoi(value);
	if (verbose) err_debug("%d\n", new->num_count_diversity);
	break;
      case DIVERSITY_TYPE_NA:
	new->diversity_type_NA = atoi(value);
	if (verbose) err_debug("%d\n", new->diversity_type_NA);
	break;
      case DIVERSITY_TYPE_A1:
	new->diversity_type_A1 = atoi(value);
	if (verbose) err_debug("%d\n", new->diversity_type_A1);
	break;
      case DIVERSITY_TYPE_A2:
	new->diversity_type_A2 = atoi(value);
	if (verbose) err_debug("%d\n", new->diversity_type_A2);
	break;
      case DIVERSITY_TYPE_A3:
	new->diversity_type_A3 = atoi(value);
	if (verbose) err_debug("%d\n", new->diversity_type_A3);
	break;
      default:
	err_warning("prob_project", "should not have reached here\n");
	break;
      }
    }

  read_attach_diversity(new);
  read_count_diversity(new);
  read_top_labels(new);
  new->postagdict = read_postag_dict(new->posdict_file, POSDICT_CACHE_LEN);
  new->unk_postagdict = read_postag_dict(new->unknown_posdict_file, UNK_POSDICT_CACHE_LEN);
  if (verbose) err_debug("opening counts db...\n");
  new->counts = (void *)prob_counts_db_open(new->counts_DB_file);
  if (verbose) err_debug("initializing cache...\n");
  new->cache = hash_new(prob_hash_sz(PROB_CACHE_LEN));
  return(new);
}

void
prob_delete (prob_t *p)
{
  int verbose = 0;

  if (!err_be_quiet()) { 
    verbose = 1;
  }

  if (verbose) err_debug("closing counts db...\n");
  prob_counts_db_close(p->counts);

  if (verbose) err_debug("deleting postag_dict...\n");
  hash_delete_with_key_delete(p->postagdict, postaglist_delete);

  if (verbose) err_debug("deleting unk_postag_dict...\n");
  hash_delete_with_key_delete(p->unk_postagdict, postaglist_delete);

  if (verbose) err_debug("deleting cache...\n");
  hash_delete(p->cache, prob_cache_delete);

  mem_free(p->attach_diversity);
  mem_free(p->count_diversity);
  mem_free(p->top_labels);

  mem_free(p->counts_DB_file);
  mem_free(p->counts_separator);
  mem_free(p->counts_NA_string);
  mem_free(p->attach_diversity_file);
  mem_free(p->toplabels_file);
  mem_free(p->posdict_file);
  mem_free(p->unknown_posdict_file);
  mem_free(p->count_diversity_file);

  mem_free(p);
}

int
prob_get_count (prob_t *p, int num, ...)
{
  char databuf[NODE_BUFLEN];
  char keybuf[COUNTS_BUFLEN];
  va_list ap;
  int i, n;

  /* get variable-length list of strings */ 
  va_start(ap, num);
  strcpy(keybuf, "");
  for (i = 0; i < num; i++) {
    /* concatenate into keybuf */
    if (strcmp(keybuf, "") != 0) {
      strcat(keybuf, p->counts_separator);
    }
    strcat(keybuf, (char *)va_arg(ap, char *));
  }
  va_end(ap);

  /* check cache */

  /* get counts from db handler */
  prob_counts_db(p->counts, keybuf, databuf);
  n = prob_conv_to_int(databuf);
    
#ifdef DEBUG_PROB
  err_debug("prob_get_count: %s\n", keybuf);
  err_debug("prob_get_count: value=%d\n", n);
#endif

  return(n);
}

void
prob_join_together (prob_t *p, char *keybuf, int num, ...)
{
  va_list ap;
  int i;

  /* get variable-length list of strings */ 
  va_start(ap, num);
  strcpy(keybuf, "");
  for (i = 0; i < num; i++) {
    /* concatenate into keybuf */
    if (strcmp(keybuf, "") != 0) {
      strcat(keybuf, p->counts_separator);
    }
    strcat(keybuf, (char *)va_arg(ap, char *));
  }
  va_end(ap);
}

double
prob_attach (prob_t *p, const char *btree, const char *bpostag, const char *bword, 
	     int node, const char *tree, const char *postag, const char *word)
{
  char nodebuf[NODE_BUFLEN];
  char keybuf[COUNTS_BUFLEN];
  int c1, c2, c1_b, c2_b;
  int c3, c4, c3_b, c4_b;
  int c5, c6, c5_b, c6_b;
  double a1_logpr, a2_logpr, a3_logpr;
  double logpr;
  probcache_t *found, *new;

  myassert(p);
  default_assign(btree);
  default_assign(bpostag);
  default_assign(bword);
  default_assign(tree);
  default_assign(postag);
  default_assign(word);

  sprintf(nodebuf, "%d", node);

  /* check cache */
  prob_join_together(p, keybuf, 7, btree, bpostag, bword, nodebuf, tree, postag, word);
  if ((found = hash_lookup(p->cache, keybuf)) != 0) {
    return(found->value);
  }

  c1 = prob_get_count(p, 5, nodebuf, tree, word, postag, btree);
  c1_b = prob_get_count(p, 4, nodebuf, tree, postag, btree);
  c2 = prob_get_count(p, 4, nodebuf, tree, word, postag);
  c2_b = prob_get_count(p, 3, nodebuf, tree, postag);

  c3 = prob_get_count(p, 6, btree, nodebuf, tree, word, postag, bpostag);
  c3_b = prob_get_count(p, 5, btree, nodebuf, tree, postag, bpostag);
  c4 = c1;
  c4_b = c1_b;

  c5 = prob_get_count(p, 7, bpostag, btree, nodebuf, tree, word, postag, bword);
  c5_b = prob_get_count(p, 6, bpostag, btree, nodebuf, tree, postag, bword);
  c6 = c3;
  c6_b = c3_b;

  a1_logpr = prob_compute(p, p->diversity_type_A1, node, c1, c2, c1_b, c2_b);
  a2_logpr = prob_compute(p, p->diversity_type_A2, node, c3, c4, c3_b, c4_b);
  a3_logpr = prob_compute(p, p->diversity_type_A3, node, c5, c6, c5_b, c6_b);
  logpr = a1_logpr + a2_logpr + a3_logpr;

#ifdef DEBUG_PROB
  err_debug("prob_attach: a1_logpr=%f\n", a1_logpr);
  err_debug("prob_attach: a2_logpr=%f\n", a2_logpr);
  err_debug("prob_attach: a3_logpr=%f\n", a3_logpr);
  err_debug("prob_attach: logpr=%f\n", logpr);
#endif

  myassert(logpr <= 0);

  /* save logpr in cache */
  new = prob_cache_new (keybuf, logpr);
  hash_enter(p->cache, keybuf, new);

  return(logpr);
}

double
prob_NA (prob_t *p, int node, const char *tree, const char *postag, const char *word)
{
  char nodebuf[NODE_BUFLEN];
  char keybuf[COUNTS_BUFLEN];
  int n, d;     /* numerator, denominator for prob_compute */
  int n_b, d_b; /* backoff values of numerator, denominator */
  double logpr;
  probcache_t *found, *new;
  
  myassert(p);
  default_assign(tree);
  default_assign(postag);
  default_assign(word);

  sprintf(nodebuf, "%d", node);

  /* check cache */
  prob_join_together(p, keybuf, 4, nodebuf, tree, postag, word);
  if ((found = hash_lookup(p->cache, keybuf)) != 0) {
    return(found->value);
  }

  n = prob_get_count(p, 5, nodebuf, tree, word, postag, p->counts_NA_string);
  d = prob_get_count(p, 4, nodebuf, tree, word, postag);
  n_b = prob_get_count(p, 4, nodebuf, tree, postag, p->counts_NA_string);
  d_b = prob_get_count(p, 3, nodebuf, tree, postag);

  logpr = prob_compute(p, p->diversity_type_NA, node, n, d, n_b, d_b);

#ifdef DEBUG_PROB
  err_debug("prob_NA: logpr=%f\n", logpr);
#endif

  myassert(logpr <= 0);

  /* save logpr in cache */
  new = prob_cache_new (keybuf, logpr);
  hash_enter(p->cache, keybuf, new);

  return(logpr);
}

double
prob_prior (prob_t *p, const char *tree, const char *postag, const char *word)
{
  int n_1, d_1, n_2, d_2;     /* numerator, denominator */
  double p1_logpr, p2_logpr;
  double logpr;
  char keybuf[COUNTS_BUFLEN];
  probcache_t *found, *new;

  myassert(p);
  default_assign(tree);
  default_assign(postag);
  default_assign(word);

  /* check cache */
  prob_join_together(p, keybuf, 3, tree, word, postag);
  if ((found = hash_lookup(p->cache, keybuf)) != 0) {
    return(found->value);
  }

  n_1 = prob_get_count(p, 3, tree, word, postag);
  d_1 = prob_get_count(p, 2, word, postag);
  n_2 = d_1;
  d_2 = p->word_postag_total;

#ifdef DEBUG_PROB
  err_debug("prob_prior: tree=%s, word=%s, postag=%s\n", tree, word, postag);
  err_debug("prob_prior: n_1=%d, d_1=%d, n_2=%d, d_2=%d\n", n_1, d_1, n_2, d_2);
#endif

  if (d_2 == 0) {
    err_abort("prob_prior", "countsWordPostagTotal value should not be zero\n");
  }

  if ( (n_1 == 0) || (d_1 == 0) ) {
    err_debug("prob_prior: did not find %s in counts db\n", keybuf);
    logpr = MIN_LOGPROB;

  } else {

    p1_logpr = log(n_1 / d_1);
    p2_logpr = log(n_2 / d_2);
    logpr = p1_logpr + p2_logpr;

  }

  myassert(logpr <= 0);

  /* save logpr in cache */
  new = prob_cache_new (keybuf, logpr);
  hash_enter(p->cache, keybuf, new);

  return(logpr);
}

double
prob_init (prob_t *p, int label)
{
  unsigned sz;
  double logpr;

  myassert(p);
  myassert(p->top_labels);

  sz = gram_num_labels(p->gram);
  myassert(label < sz);

  logpr = (prob_toplabel_count(p, label) + PROB_ALPHA) /  
    ((p->total_top) + (sz * PROB_ALPHA)) ;

  myassert(logpr <= 0);
  return(logpr);
}

int
prob_conv_to_int (const char *databuf)
{
  return((strcmp(databuf, "") == 0) ? 0 : atoi(databuf));
}

double
prob_compute_lambda (prob_t *p, int type, int c)
{
  return(c / (c + prob_get_count_diversity(p, type)));
}

double
prob_compute (prob_t *p, int type, int node, int n, int d, int n_b, int d_b)
{
  int label;
  double lambda;
  double e1, e2;
  int k; /* attach diversity */

#ifdef DEBUG_PROB
  err_debug("prob_compute: n=%d, d=%d\n", n, d);
  err_debug("prob_compute: n_b=%d, d_b=%d\n", n_b, d_b);
#endif

  myassert(d ? 1 : !n);
  myassert(d_b ? 1 : !n_b);

  label = gram_label_index(p->gram, node);
  myassert(label < gram_num_labels(p->gram));
  k = p->attach_diversity[label];

  e1 = (n + PROB_ALPHA) / 
    (d + (k * PROB_ALPHA));

  e2 = (n_b + PROB_ALPHA) / 
    (d_b + (k * PROB_ALPHA));

  lambda = prob_compute_lambda(p, type, n);

  return(log( (lambda * e1) + ((1 - lambda) * e2)) );
}

int
prob_get_count_diversity (prob_t *p, int type)
{
  unsigned i, sz;

  sz = p->num_count_diversity;
  for (i = 0; i < sz; i++)
    {
      if (p->count_diversity[i].type == type) {
	return(p->count_diversity[i].count);
      }
    }

  err_abort("prob_get_count_diversity", 
	    "unknown type: %d\n",
	    type);
  return(0);
}


void
read_attach_diversity (prob_t *p)
{
  FILE *diver_f;
  char line[PROB_LINE_LEN];
  const char *filename;
  unsigned i, sz;
  int label, diversity;

  myassert(p);

  filename = p->attach_diversity_file;
  sz = gram_num_labels(p->gram);

  p->attach_diversity = (int *) mem_alloc(sz * sizeof(int));

  for (i = 0; i < sz; i++)
    {
      p->attach_diversity[i] = 0;
    }

  if ((diver_f = fopen(filename, "r")) == NULL) {
    err_abort("read_attach_diversity", 
	      "could not open attach diversity file %s\n", 
	      filename);
  }

  i = 0;
  while ((fgets(line, PROB_LINE_LEN, diver_f)))
    {
      if (feof(diver_f)) { break; }
      sscanf(line, "%d %d\n", &label, &diversity);
      myassert(label < sz);
      p->attach_diversity[label] = diversity;
      i++;
    }

  (void) fclose(diver_f);
  err_debug("loaded %d items from %s\n", i, filename);
}

void
read_count_diversity (prob_t *p)
{
  FILE *diver_f;
  char line[PROB_LINE_LEN];
  const char *filename;
  unsigned i, sz;
  int count, type;

  myassert(p);

  filename = p->count_diversity_file;
  sz = p->num_count_diversity;

  p->count_diversity = (countd_t *) mem_alloc(sz * sizeof(countd_t));

  for (i = 0; i < sz; i++)
    {
      p->count_diversity[i].count = 0;
      p->count_diversity[i].type = 0;
    }

  if ((diver_f = fopen(filename, "r")) == NULL) {
    err_abort("read_count_diversity", 
	      "could not open count diversity file %s\n", 
	      filename);
  }

  i = 0;
  while ((fgets(line, PROB_LINE_LEN, diver_f)))
    {
      if (feof(diver_f)) { break; }
      sscanf(line, "%d %d\n", &count, &type);
      myassert(i < p->num_count_diversity);
      p->count_diversity[i].count = count;
      p->count_diversity[i].type = type;
      i++;
    }

  (void) fclose(diver_f);
  err_debug("loaded %d items from %s\n", i, filename);
}

void
read_top_labels (prob_t *p)
{
  FILE *toplabels_f;
  char line[PROB_LINE_LEN];
  const char *filename;
  unsigned i, sz;
  unsigned total_top = 0;
  int label, count;

  myassert(p);

  filename = p->toplabels_file;
  sz = gram_num_labels(p->gram);

  p->top_labels = (int *) mem_alloc(sz * sizeof(int));

  for (i = 0; i < sz; i++)
    {
      p->top_labels[i] = 0;
    }

  if ((toplabels_f = fopen(filename, "r")) == NULL) {
    err_abort("read_top_labels", 
	      "could not open attach diversity file %s\n", 
	      filename);
  }

  i = 0;
  while ((fgets(line, PROB_LINE_LEN, toplabels_f)))
    {
      if (feof(toplabels_f)) { break; }
      sscanf(line, "%d %d\n", &label, &count);
      myassert(label < sz);
      p->top_labels[label] = count;
      total_top += count;
      i++;
    }
  p->total_top = total_top;

  (void) fclose(toplabels_f);
  err_debug("loaded %d items from %s\n", i, filename);
}

int
prob_toplabel_count (prob_t *p, int idx)
{
  return(p->top_labels[idx]);
}

hash_t *
read_postag_dict (const char *filename, int len)
{
  FILE *postag_f;
  char word[PROB_LINE_LEN], postag[PROB_LINE_LEN];
  char *key;
  unsigned i, sz, tot;
  part_of_speech_t *postaglist;
  int fstatus;
  hash_t *new;

  new = hash_new(prob_hash_sz(len));
  if ((postag_f = fopen(filename, "r")) == NULL) {
    err_abort("read_postag_dict", 
	      "could not open postag dict file %s\n", 
	      filename);
  }

  tot = 0;
  for(;;)
    {
      if (feof(postag_f)) { break; }
      fstatus = fscanf(postag_f, "%s %d", word, &sz);
      if (fstatus == EOF) { break; }
      postaglist = 0; 
      for (i = 0; i < sz; i++)
	{
	  fstatus = fscanf(postag_f, "%s", postag);
	  if (fstatus == EOF) { 
	    err_abort("read_postag_dict", 
		      "postag_dict file %s too short at line %d\n", 
		      filename, i+1);
	  }
	  if (postaglist == 0) {
	    postaglist = postaglist_new(postag);
	  } else {
	    postaglist = postaglist_add(postaglist, postag);
	  }
	}
      if (hash_lookup(new, word)) {
	err_warning("read_postag_dict", 
		    "entered %s already, skipping...\n", word);
      } else {
	key = mem_strdup(word);
	hash_enter(new, key, postaglist);
	tot++;
      }
    }
  (void) fclose(postag_f);
  err_debug("loaded %d items from %s\n", tot, filename);
  return(new);
}

part_of_speech_t *
postag_lookup (prob_t *p, const char *word)
{
  part_of_speech_t *postaglist;
  postaglist = (part_of_speech_t *)hash_lookup(p->postagdict, word);
  if (postaglist == 0) {
    postaglist = (part_of_speech_t *)hash_lookup(p->unk_postagdict, word);
  }
  if (postaglist == 0) {
    err_warning("postag_lookup", 
		"could not find %s in tag dictionary\n", 
		word);
  }
  return(postaglist);
}

probcache_t *
prob_cache_new (const char *key, double logprob)
{
  probcache_t *new;

  new = (probcache_t *) mem_alloc(sizeof(probcache_t));
  new->key = mem_strdup(key);
  new->value = logprob;
  return(new);
}

void
prob_cache_delete (void *v)
{
  probcache_t *p;
  if (v) {
    p = (probcache_t *)v;
    if (p->key) { mem_free(p->key); }
    mem_free(v); 
  }
}

/* cp prob.c probtest.c
   <make changes to remove gram dependencies>
   alias compile='gcc -o probtest -DDEBUG_MEM err.c mem.c hash.c probtest.c -lm' 
*/
/*
int
main (int argc, char **argv)
{
  prob_t *p;

#ifdef DEBUG_MEM
  mem_debug_init();
#endif

  p = prob_new();
  prob_delete(p);

#ifdef DEBUG_MEM
  mem_debug_close();
  mem_report();
#endif

  return(0);
}
*/
