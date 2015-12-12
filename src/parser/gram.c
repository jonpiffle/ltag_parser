
/* gram - read and create grammar data structures */

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

static char rcsid[] = "$Id: gram.c,v 1.10 2000/10/05 23:18:48 anoop Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myassert.h"
#include "gram.h"
#include "gram_project.h"
#include "mem.h"
#include "err.h"

#ifdef USE_BERKELEY_DB
#include "syn_db.h"
#else
#include "syn_hash.h"
#endif

grammar_t *
gram_new (const char *filename)
{
  FILE *project_f;
  int tag_index, linenum, fstatus = 0;
  char line[PRJ_LINELEN];
  char *attr, *value;
  grammar_t *new;

  if ((project_f = fopen(filename, "r")) == NULL) {
    err_abort("gram_new", "could not open grammar file %s\n", filename);
  }
  new = (grammar_t *) mem_alloc(sizeof(grammar_t));
  new->modelfile = 0;
  linenum = 0;
  while ((fstatus = fscanf(project_f, "%s\n", line)))
    {
      if (fstatus == EOF) break;
      linenum++;
      project_linesplit(line, &attr, &value);
      if ((tag_index = project_getindex(attr)) == 0) {
	err_warning("gram_new", "syntax error in %s line %d\n", filename, linenum);
	continue;
      }
      switch(tag_index) {
      case NUMTREES:
	new->num_trees = atoi(value);
	break;
      case NUMFAMILIES:
	new->num_families = atoi(value);
	break;
      case NUMTREENODES:
	new->num_treenodes = atoi(value);
	break;
      case NUMLABELS:
	new->num_labels = atoi(value);
	break;
      case NUMSUBSCRIPTS:
	new->num_subscripts = atoi(value);
	break;
      case WIDTHLABELS:
	new->width_labels = atoi(value);
	break;
      case WIDTHSUBSCRIPTS:
	new->width_subscripts = atoi(value);
	break;
      case ELEMENTARYTREES:
	new->elementary_trees = mem_strdup(value);
	break;
      case TREENAMES:
	new->tree_names = mem_strdup(value);
	break;
      case TREENODES:
	new->tree_nodes = mem_strdup(value);
	break;
      case TREEFEATURES:
	new->tree_features = mem_strdup(value);
	break;
      case TREEFAMILIES:
	new->tree_families = mem_strdup(value);
	break;
      case TREELABELS:
	new->tree_labels = mem_strdup(value);
	break;
      case TREESUBSCRIPTS:
	new->tree_subscripts = mem_strdup(value);
	break;
      case WORD_DB:
	new->dbfile = mem_strdup(value);
	break;
      case NUM_EMPTY:
	new->num_eps = atoi(value);
	break;
      case EMPTY_ELTS:
	new->epsilons = mem_strdup(value);
	break;
      case NUM_TOPLABELS:
	new->num_toplabels = atoi(value);
	break;
      case TOPLABELS:
	new->toplabels = mem_strdup(value);
	break;
      case SYNDEFAULT:
	new->default_file = mem_strdup(value);
	break;
      case MODELFILE:
	new->modelfile = mem_strdup(value);
	break;
      default:
	err_warning("gram_new", "syntax error in %s line %d\n", filename, linenum);
	break;
      }
    }
  (void) fclose(project_f);

#ifdef DEBUG_PROJECT
    project_debug(new);
#endif

  read_labels(new);
  read_subscripts(new);
  read_treenodes(new);
  read_trees(new);
  read_treenames(new);
  read_treefeatures(new);
  read_families(new);
  read_epsilons(new);
  read_toplabels(new);
  read_syndefault(new);

  myassert(new->dbfile);

#ifdef USE_BERKELEY_DB
  new->dbp = (void *)syn_db_open(new->dbfile);
#else
  new->dbp = (void *)syn_hash(new->dbfile);
#endif

  gram_treeinfo(new);

  return(new);
}

void 
gram_delete (grammar_t *gram)
{
  myassert(gram);
  myassert(gram->dbp);

#ifdef USE_BERKELEY_DB
  syn_db_close(gram->dbp);
#else
  hash_delete((hash_t *)gram->dbp, syn_hashentry_delete);
#endif

  gram_delete_treeinfo(gram);

  delete_labels(gram);
  delete_subscripts(gram);
  delete_treenodes(gram);
  delete_trees(gram);
  delete_treenames(gram);
  delete_treefeatures(gram);
  delete_families(gram);
  delete_epsilons(gram);
  delete_toplabels(gram);
  delete_syndefault(gram);

  mem_free(gram->elementary_trees);
  mem_free(gram->tree_names);
  mem_free(gram->tree_nodes);
  mem_free(gram->tree_features);
  mem_free(gram->tree_families);
  mem_free(gram->tree_labels);
  mem_free(gram->tree_subscripts);
  mem_free(gram->epsilons);
  mem_free(gram->toplabels);
  mem_free(gram->default_file);
  mem_free(gram->dbfile);

  if (gram->modelfile) {
    mem_free(gram->modelfile);
  }

  mem_free(gram);
}

int 
project_getindex (char *tag)
{
  int sz = project_tags[0].index;
  int i;
  for (i = 1; i < sz; i++)
    {
      if (strcmp(tag, project_tags[i].tag) == 0) {
	return(project_tags[i].index);
      }
    }
  return(0); /* syntax error */
}

void
project_linesplit (char *line, char **attr, char **value)
{
  const char *seperator = project_tags[0].tag;
  *attr = strtok(line, seperator);
  *value = strtok(0, seperator);
  if ((*attr == NULL) || (*value == NULL)) {
    err_abort("gram_new", "syntax error. perhaps you have spaces in the file\n");
  }
}

void
project_debug (grammar_t *gram)
{
  int sz = project_tags[0].index;
  int i;
  myassert(gram);
  for (i = 1; i < sz; i++)
    {
      err_debug("%s=", project_tags[i].tag);
      switch(project_tags[i].index) {
      case NUMTREES:
	err_debug(NUMBER, gram->num_trees);
	break;
      case NUMFAMILIES:
	err_debug(NUMBER, gram->num_families);
	break;
      case NUMTREENODES:
	err_debug(NUMBER, gram->num_treenodes);
	break;
      case NUMLABELS:
	err_debug(NUMBER, gram->num_labels);
	break;
      case NUMSUBSCRIPTS:
	err_debug(NUMBER, gram->num_subscripts);
	break;
      case WIDTHLABELS:
	err_debug(NUMBER, gram->width_labels);
	break;
      case WIDTHSUBSCRIPTS:
	err_debug(NUMBER, gram->width_subscripts);
	break;
      case ELEMENTARYTREES:
	err_debug(FILENAME, gram->elementary_trees);
	break;
      case TREENAMES:
	err_debug(FILENAME, gram->tree_names);
	break;
      case TREENODES:
	err_debug(FILENAME, gram->tree_nodes);
	break;
      case TREEFEATURES:
	err_debug(FILENAME, gram->tree_features);
	break;
      case TREEFAMILIES:
	err_debug(FILENAME, gram->tree_families);
	break;
      case TREELABELS:
	err_debug(FILENAME, gram->tree_labels);
	break;
      case TREESUBSCRIPTS:
	err_debug(FILENAME, gram->tree_subscripts);
	break;
      case WORD_DB:
	err_debug(FILENAME, gram->dbfile);
	break;
      case NUM_EMPTY:
	err_debug(NUMBER, gram->num_eps);
	break;
      case EMPTY_ELTS:
	err_debug(FILENAME, gram->epsilons);
	break;
      case NUM_TOPLABELS:
	err_debug(NUMBER, gram->num_toplabels);
	break;
      case TOPLABELS:
	err_debug(FILENAME, gram->toplabels);
	break;
      case SYNDEFAULT:
	err_debug(FILENAME, gram->default_file);
	break;
      case MODELFILE:
	if (gram->modelfile) { 
           err_debug(FILENAME, gram->modelfile);
        } else {
           err_debug("none\n");
        }
	break;
      default:
	err_warning("project_debug", "should not have reached here\n");
	break;
      }
    }
}

int
gram_toplabel_index (grammar_t *g, int label)
{
  int sz, i;

  myassert(g);
  sz = g->num_toplabels;
  for (i = 0; i < sz; i++)
    {
      if (g->toplabel_tbl[i].toplabel == label) {
	return(i);
      }
    }
  return(-1);
}

