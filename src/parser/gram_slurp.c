
/* gram_slurp - functions that read the grammar files */

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

static char rcsid[] = "$Id: gram_slurp.c,v 1.4 2000/10/23 19:15:56 anoop Exp $";

#include <stdio.h>
#include <string.h>
#include "myassert.h"
#include "gram.h"
#include "mem.h"
#include "err.h"

void
read_labels(grammar_t *g)
{
  FILE *labels_f;
  int i, sz, width, fstatus;

  myassert(g);

  if ((labels_f = fopen(g->tree_labels, "r")) == NULL) {
    err_abort("read_labels", 
	      "could not open label file %s\n", 
	      g->tree_labels);
  }

  sz = g->num_labels;
  width = g->width_labels+1;
  g->label_tbl = (char **) mem_alloc(sz * sizeof(char *));

  for (i = 0; i < sz; i++)
    {
      g->label_tbl[i] = (char *) mem_alloc(width * sizeof(char));
      fstatus = fscanf(labels_f, "%s\n", g->label_tbl[i]);

      if (fstatus == EOF) {
	err_abort("read_labels", 
		  "label file %s too short at line %d\n", 
		  g->tree_labels, i+1);
      }

    }

  (void) fclose(labels_f);
}

void
delete_labels(grammar_t *g)
{
  int i, sz;
  myassert(g);

  sz = g->num_labels;

  for (i = 0; i < sz; i++)
    {

#ifdef DEBUG_LABELS
      err_debug("%s\n", g->label_tbl[i]);
#endif

      mem_free(g->label_tbl[i]);
    }  

  mem_free(g->label_tbl);
}

void
read_subscripts(grammar_t *g)
{
  FILE *subscripts_f;
  int i, sz, width, fstatus;

  myassert(g);

  if ((subscripts_f = fopen(g->tree_subscripts, "r")) == NULL) {
    err_abort("read_subscripts", 
	      "could not open subscript file %s\n", 
	      g->tree_subscripts);
  }

  sz = g->num_subscripts;
  width = g->width_subscripts+1;
  g->subscript_tbl = (char **) mem_alloc(sz * sizeof(char *));

  for (i = 0; i < sz; i++)
    {
      g->subscript_tbl[i] = (char *) mem_alloc(width * sizeof(char));
      fstatus = fscanf(subscripts_f, "%s\n", g->subscript_tbl[i]);

      if (fstatus == EOF) {
	err_abort("read_subscripts", 
		  "subscript file %s too short at line %d\n", 
		  g->tree_subscripts, i+1);
      }

    }

  (void) fclose(subscripts_f);
}

void
delete_subscripts(grammar_t *g)
{
  int i, sz;
  myassert(g);

  sz = g->num_subscripts;

  for (i = 0; i < sz; i++)
    {

#ifdef DEBUG_SUBSCRIPTS
      err_debug("%s\n", g->subscript_tbl[i]);
#endif

      mem_free(g->subscript_tbl[i]);
    }  

  mem_free(g->subscript_tbl);
}

void 
read_treenodes (grammar_t *g)
{
  FILE *treenodes_f;
  int i, sz, fstatus;
  int leftchild, right, flag, label, subscript;

  myassert(g);

  if ((treenodes_f = fopen(g->tree_nodes, "r")) == NULL) {
    err_abort("read_treenodes", 
	      "could not open treenodes file %s\n", 
	      g->tree_nodes);
  }

  sz = g->num_treenodes;
  g->treenode_tbl = (treenode_t *) mem_alloc(sz * sizeof(treenode_t));
  g->inverted_subst_tbl = (int *) mem_alloc(sz * sizeof(int));
  g->inverted_foot_tbl = (int *) mem_alloc(sz * sizeof(int));
  g->inverted_anchor_tbl = (int *) mem_alloc(sz * sizeof(int));
  g->inverted_NA_tbl = (int *) mem_alloc(sz * sizeof(int));

  for (i = 0; i < sz; i++)
    {
      fstatus = fscanf(treenodes_f, TREENODES_FORMAT, 
			   &leftchild, &right, &flag, &label, &subscript);

      if (fstatus == EOF) {
	err_abort("read_treenodes", 
		  "treenodes file %s too short at line %d\n", 
		  g->tree_nodes, i+1);
      }

      g->treenode_tbl[i].leftchild = leftchild;
      g->treenode_tbl[i].right = right;
      g->treenode_tbl[i].flag = flag;
      g->treenode_tbl[i].label = label;
      g->treenode_tbl[i].subscript = subscript;

      g->inverted_subst_tbl[i] =
	g->inverted_foot_tbl[i] =
	g->inverted_anchor_tbl[i] =
	g->inverted_NA_tbl[i] = NULL_T;

#ifdef DEBUG_TREENODES
      err_debug(TREENODES_FORMAT, 
		g->treenode_tbl[i].leftchild, 
		g->treenode_tbl[i].right, 
		g->treenode_tbl[i].flag, 
		g->treenode_tbl[i].label, 
		g->treenode_tbl[i].subscript);
#endif

    }

  (void) fclose(treenodes_f);
  if (!err_be_quiet()) {
    err_debug("loaded %d treenodes from %s\n", sz, g->tree_nodes);
  }
}  

void 
delete_treenodes (grammar_t *g)
{
  myassert(g);

  mem_free(g->treenode_tbl);
  mem_free(g->inverted_subst_tbl);
  mem_free(g->inverted_foot_tbl);
  mem_free(g->inverted_anchor_tbl);
  mem_free(g->inverted_NA_tbl);
}

void 
read_trees (grammar_t *g)
{
  FILE *trees_f;
  int i, j, sz, fstatus, treenode;
  int root, foot, num_subst, num_anchor, num_NA, num_treenodes;

  myassert(g);

  if ((trees_f = fopen(g->elementary_trees, "r")) == NULL) {
    err_abort("read_trees", 
	      "could not open trees file %s\n", 
	      g->elementary_trees);
  }

  sz = g->num_trees;
  num_treenodes = g->num_treenodes;

  g->tree_tbl = (int *) mem_alloc(sz * sizeof(int));
  g->footnode_tbl = (int *) mem_alloc(sz * sizeof(int));
  g->num_subst_tbl = (int *) mem_alloc(sz * sizeof(int));
  g->num_anchor_tbl = (int *) mem_alloc(sz * sizeof(int));
  g->num_NA_tbl = (int *) mem_alloc(sz * sizeof(int));

  g->subst_list_tbl = (int **) mem_alloc(sz * sizeof(int *));
  g->anchor_list_tbl = (int **) mem_alloc(sz * sizeof(int *));
  g->NA_list_tbl = (int **) mem_alloc(sz * sizeof(int *));

  for (i = 0; i < sz; i++)
    {
      fstatus = fscanf(trees_f, TREES_FORMAT,
		       &root, &foot, &num_subst, &num_anchor, &num_NA);

      if (fstatus == EOF) {
	err_abort("read_trees", 
		  "tree file %s too short at line %d\n", 
		  g->elementary_trees, i+1);
      }

#ifdef DEBUG_TREES
      err_debug(TREES_FORMAT, root, foot, num_subst, num_anchor, num_NA);
#endif
      
      g->tree_tbl[i] = root;
      g->footnode_tbl[i] = foot;
      g->num_subst_tbl[i] = num_subst;
      g->num_anchor_tbl[i] = num_anchor;
      g->num_NA_tbl[i] = num_NA;

      myassert(g->inverted_foot_tbl);
      myassert(foot < num_treenodes);
      if (foot >= 0) {
          g->inverted_foot_tbl[foot] = ((foot == -1) ? -1 : i);
      }

      /* read nodes marked for substitution in tree i */
      if (num_subst > 0) 
	{
	  int subst_sz = num_subst;
	  g->subst_list_tbl[i] = (int *) mem_alloc(subst_sz * sizeof(int));

	  for (j = 0; j < subst_sz; j++) 
	    {
	      fstatus = fscanf(trees_f, LIST_FORMAT, &treenode);

	      if (fstatus == EOF) {
		err_abort("read_trees", 
			  "tree file %s too short at line %d\n", 
			  g->elementary_trees, i+1);
	      }

	      myassert(treenode > NULL_T);
	      myassert(treenode < num_treenodes);
	      g->subst_list_tbl[i][j] = treenode;

	      myassert(g->inverted_subst_tbl);
	      g->inverted_subst_tbl[treenode] = i;

#ifdef DEBUG_TREES
	      err_debug(" %d", g->subst_list_tbl[i][j]);
#endif
	    }
	}
      else 
	{
	  g->subst_list_tbl[i] = 0;
	}

      /* read nodes marked as anchors in tree i */
      if (num_anchor > 0) 
	{
	  int anchor_sz = num_anchor;
	  g->anchor_list_tbl[i] = (int *) mem_alloc(anchor_sz * sizeof(int));

	  for (j = 0; j < anchor_sz; j++) 
	    {
	      fstatus = fscanf(trees_f, LIST_FORMAT, &treenode);

	      if (fstatus == EOF) {
		err_abort("read_trees", 
			  "tree file %s too short at line %d\n", 
			  g->elementary_trees, i+1);
	      }

	      myassert(treenode > NULL_T);
	      myassert(treenode < num_treenodes);
	      g->anchor_list_tbl[i][j] = treenode;

	      myassert(g->inverted_anchor_tbl);
	      g->inverted_anchor_tbl[treenode] = i;

#ifdef DEBUG_TREES
	      err_debug(" %d", g->anchor_list_tbl[i][j]);
#endif
	    }
	} 
      else 
	{
	  g->anchor_list_tbl[i] = 0;
	}

      /* read nodes marked as null adjunction in tree i */
      if (num_NA > 0) 
	{
	  int NA_sz = num_NA;
	  g->NA_list_tbl[i] = (int *) mem_alloc(NA_sz * sizeof(int));

	  for (j = 0; j < NA_sz; j++) 
	    {
	      fstatus = fscanf(trees_f, LIST_FORMAT, &treenode);

	      if (fstatus == EOF) {
		err_abort("read_trees", 
			  "tree file %s too short at line %d\n", 
			  g->elementary_trees, i+1);
	      }

	      myassert(treenode > NULL_T);
	      myassert(treenode < num_treenodes);
	      g->NA_list_tbl[i][j] = treenode;

	      myassert(g->inverted_NA_tbl);
	      g->inverted_NA_tbl[treenode] = i;

#ifdef DEBUG_TREES
	      err_debug(" %d", g->NA_list_tbl[i][j]);
#endif

	    }
	}
      else 
	{
	  g->NA_list_tbl[i] = 0;
	}

      if (fgetc(trees_f) != '\n') {
	err_abort("read_trees", 
		  "missing new line in %s at line %d\n", 
		  g->elementary_trees, i+1);
      }

#ifdef DEBUG_TREES
      err_debug("\n");
#endif

    }
  (void) fclose(trees_f);
  if (!err_be_quiet()) {
    err_debug("loaded %d trees from %s\n", sz, g->elementary_trees);
  }
}


void
delete_trees (grammar_t *g)
{
  int i, sz;

  myassert(g);
  sz = g->num_trees;

  for (i = 0; i < sz; i++)
    {
      if (g->subst_list_tbl[i]) mem_free(g->subst_list_tbl[i]);
      if (g->anchor_list_tbl[i]) mem_free(g->anchor_list_tbl[i]);
      if (g->NA_list_tbl[i]) mem_free(g->NA_list_tbl[i]);
    }
  mem_free(g->subst_list_tbl);
  mem_free(g->anchor_list_tbl);
  mem_free(g->NA_list_tbl);

  mem_free(g->tree_tbl);
  mem_free(g->footnode_tbl);
  mem_free(g->num_subst_tbl);
  mem_free(g->num_anchor_tbl);
  mem_free(g->num_NA_tbl);
}

void 
read_treenames (grammar_t *g)
{
  FILE *treenames_f;
  int i, sz;
  char *fstatus;
  char line[TREENAME_LEN];

  myassert(g);

  if ((treenames_f = fopen(g->tree_names, "r")) == NULL) {
    err_abort("read_treenames", 
	      "could not open treename file %s\n", 
	      g->tree_names);
  }

  sz = g->num_trees;
  g->treename_tbl = (char **) mem_alloc(sz * sizeof(char *));

  for (i = 0; i < sz; i++)
    {
      fstatus = fgets(line, TREENAME_LEN, treenames_f);

      if (fstatus == NULL) {
	err_abort("read_treenames", 
		  "treename file %s too short at line %d\n", 
		  g->tree_names, i+1);
      }

      line[(int) strlen(line)-1] = (char) '\0';  /* remove the newline character */
      g->treename_tbl[i] = (char *) mem_alloc((strlen(line)+1) * sizeof(char));
      (void) strcpy(g->treename_tbl[i], line);

#ifdef DEBUG_TREENAMES
      err_debug("%s\n", g->treename_tbl[i]);
#endif

    }
  (void) fclose(treenames_f);
  if (!err_be_quiet()) {
    err_debug("loaded %d treenames from %s\n", sz, g->tree_names);
  }
}

void 
delete_treenames (grammar_t *g)
{
  int i, sz;
  
  myassert(g);
  sz = g->num_trees;

  for (i = 0; i < sz; i++)
    {
      if (g->treename_tbl[i]) mem_free(g->treename_tbl[i]);
    }
  mem_free(g->treename_tbl);
}

void 
read_treefeatures (grammar_t *g)
{
  FILE *treefeatures_f;
  int i, sz;
  char *fstatus;
  char line[FEAT_LINELEN];

  myassert(g);

  if ((treefeatures_f = fopen(g->tree_features, "r")) == NULL) {
    err_abort("read_treefeatures", 
	      "could not open treefeatures file %s\n", 
	      g->tree_features);
  }

  sz = g->num_trees;
  g->treefeat_tbl = (char **) mem_alloc(sz * sizeof(char *));

  for (i = 0; i < sz; i++)
    {
      fstatus = fgets(line, FEAT_LINELEN, treefeatures_f);
      line[strlen(line)-1] = (char) '\0';  /* remove the newline character */

      if (fstatus == NULL) {
	err_abort("read_treefeatures", 
		  "features file %s too short at line %d\n", 
		  g->tree_features, i+1);
      }

      g->treefeat_tbl[i] = (char *) mem_alloc((strlen(line)+1) * sizeof(char));
      (void) strcpy(g->treefeat_tbl[i], line);

#ifdef DEBUG_TREEFEATURES
      err_debug("%s\n", g->treefeat_tbl[i]);
#endif

    }
  (void) fclose(treefeatures_f);
  if (!err_be_quiet()) {
    err_debug("loaded %d features from %s\n", sz, g->tree_features);
  }
}

void 
delete_treefeatures (grammar_t *g)
{
  int i, sz;
  
  myassert(g);
  sz = g->num_trees;

  for (i = 0; i < sz; i++)
    {
      if (g->treefeat_tbl[i]) mem_free(g->treefeat_tbl[i]);
    }
  mem_free(g->treefeat_tbl);
}

void 
read_families (grammar_t *g)
{
  FILE *family_f;
  int i, j, sz, fstatus, family_size, tree_index;
  char family_name[FAMILYNAME_LEN], tree_name[TREENAME_LEN];

  myassert(g);

  if ((family_f = fopen(g->tree_families, "r")) == NULL) {
    err_abort("read_families", 
	      "could not open treefamilies file %s\n", 
	      g->tree_families);
  }

  sz = g->num_families;
  g->family_tbl = (char **) mem_alloc(sz * sizeof(char *));
  g->family_size = (int *) mem_alloc(sz * sizeof(char *));
  g->family_tree = (int **) mem_alloc(sz * sizeof(int *));

  for (i = 0; i < sz; i++)
    {
      fstatus = fscanf(family_f, FAMILY_FORMAT, family_name, &family_size);

      if (fstatus == EOF) {
	err_abort("read_families", 
		  "family file %s too short at line %d\n", 
		  g->tree_families, i+1);
      }

      g->family_tbl[i] = (char *) mem_alloc((strlen(family_name)+1) * sizeof(char));
      g->family_size[i] = family_size;
      g->family_tree[i] = (int *) mem_alloc(family_size * sizeof(int));

      (void) strcpy(g->family_tbl[i], family_name);

#ifdef DEBUG_FAMILIES
      err_debug("%s %d ", g->family_tbl[i], g->family_size[i]);
#endif

      for (j = 0; j < family_size; j++)
	{
	  fstatus = fscanf(family_f, "%s", tree_name);

	  if (fstatus == EOF) {
	    err_abort("read_families", 
		      "some trees missing in %s at line %d\n", 
		      g->tree_families, i+1);
	  }

	  if ((tree_index = 
	       gram_treeindex((const grammar_t *) g, tree_name)) == NULL_T) {
	    err_abort("read_families", 
		      "bad tree name %s in %s at line %d\n", 
		      tree_name, g->tree_families, i+1);
	  }

	  g->family_tree[i][j] = tree_index;

#ifdef DEBUG_FAMILIES
      err_debug("%s(%d) ", tree_name, g->family_tree[i][j]);
#endif
	}

      if (fgetc(family_f) != '\n') {
	err_abort("read_families", 
		  "missing new line in %s at line %d\n", 
		  g->tree_families, i+1);
      }

#ifdef DEBUG_FAMILIES
      err_debug("\n");
#endif

    }
  (void) fclose(family_f);
  if (!err_be_quiet()) {
    err_debug("loaded %d family definitions from %s\n", sz, g->tree_families);
  }
}

void
delete_families (grammar_t *g)
{
  int i, sz;

  myassert(g);
  sz = g->num_families;

  for (i = 0; i < sz; i++)
    {
      if (g->family_tbl[i]) mem_free(g->family_tbl[i]);
      if (g->family_tree[i]) mem_free(g->family_tree[i]);
    }
  mem_free(g->family_tbl);
  mem_free(g->family_size);
  mem_free(g->family_tree);
}

void
read_epsilons (grammar_t *g)
{
  FILE *eps_f;
  int i, fstatus, sz, label;
  char *category;

  myassert(g);

  if ((eps_f = fopen(g->epsilons, "r")) == NULL) {
    err_abort("read_epsilons", 
	      "could not open epsilon file %s\n", 
	      g->epsilons);
  }

  sz = g->num_eps;
  g->eps_tbl = (eps_t *) mem_alloc(sz * sizeof(eps_t));
  category = (char *) mem_alloc(g->width_labels * sizeof(char));

  for (i = 0; i < sz; i++)
    {
      category[0] = '\0';
      g->eps_tbl[i].empty_elt = (char *) mem_alloc(_gram_empty_elt_width * sizeof(char));
      fstatus = fscanf(eps_f, EPSILON_FORMAT, g->eps_tbl[i].empty_elt, category);

      if (fstatus == EOF) {
	err_abort("read_epsilons", 
		  "epsilons file %s too short at line %d\n", 
		  g->epsilons, i+1);
      }
      
      if ((label = gram_index_from_label(g, category)) == -1) {
	err_abort("read_epsilons", 
		  "epsilons file %s has unknown label at line %d\n", 
		  g->epsilons, i+1);
      }

      g->eps_tbl[i].label = label;

#ifdef DEBUG_EPSILONS
      err_debug("%s %s\n", 
		g->eps_tbl[i].empty_elt, 
		gram_label_from_index(g, g->eps_tbl[i].label));
#endif

    }
  mem_free(category);

  (void) fclose(eps_f);
  if (!err_be_quiet()) {
    err_debug("loaded %d empty elements from %s\n", sz, g->epsilons);
  }
}

void
delete_epsilons (grammar_t *g)
{
  int sz, i;

  myassert(g);
  sz = g->num_eps;
  for (i = 0; i < sz; i++)
    {
      mem_free(g->eps_tbl[i].empty_elt);
    }
  mem_free(g->eps_tbl);
}

void
read_toplabels (grammar_t *g)
{
  FILE *top_f;
  int i, fstatus, sz, label;
  char *category;
  char startfeat[FEAT_LINELEN];

  myassert(g);

  if ((top_f = fopen(g->toplabels, "r")) == NULL) {
    err_abort("read_toplabels", 
	      "could not open toplabels file %s\n", 
	      g->toplabels);
  }

  sz = g->num_toplabels;
  g->toplabel_tbl = (toplabel_t *) mem_alloc(sz * sizeof(toplabel_t));
  category = (char *) mem_alloc(g->width_labels * sizeof(char));

  for (i = 0; i < sz; i++)
    {
      category[0] = '\0';
      startfeat[0] = '\0';
      fstatus = fscanf(top_f, TOPLABEL_FORMAT, category, startfeat);

      if (fstatus == EOF) {
	err_abort("read_toplabels", 
		  "toplabels file %s too short at line %d\n", 
		  g->toplabels, i+1);
      }
      
      if ((label = gram_index_from_label(g, category)) == -1) {
	err_abort("read_toplabels", 
		  "toplabels file %s has unknown label at line %d\n", 
		  g->toplabels, i+1);
      }

      g->toplabel_tbl[i].toplabel = label;
      g->toplabel_tbl[i].startfeat = mem_strdup(startfeat);

#ifdef DEBUG_TOPLABELS
      err_debug("%s %s\n", 
		gram_label_from_index(g, g->toplabel_tbl[i].toplabel),
		g->toplabel_tbl[i].startfeat);
#endif

    }
  mem_free(category);

  (void) fclose(top_f);
  if (!err_be_quiet()) {
    err_debug("loaded %d toplabels from %s\n", sz, g->toplabels);
  }
}

void
delete_toplabels (grammar_t *g)
{
  int sz, i;

  myassert(g);
  sz = g->num_toplabels;
  for (i = 0; i < sz; i++)
    {
      mem_free(g->toplabel_tbl[i].startfeat);
    }
  mem_free(g->toplabel_tbl);
}

void
read_syndefault (grammar_t *g)
{
  FILE *default_f;
  char *fstatus;
  char syndefault[SYNDEFAULT_LEN];
  int idx;

  myassert(g);

  if ((default_f = fopen(g->default_file, "r")) == NULL) {
    err_abort("read_syndefault", 
	      "could not open default file %s\n", 
	      g->default_file);
  }

  g->syndefault = (char **)mem_alloc(_gram_max_default_entries * sizeof(char *));

  idx = 0;
  while (1) {

    fstatus = fgets(syndefault, SYNDEFAULT_LEN, default_f);

    if ((fstatus == NULL) || (feof(default_f))) {
      break;
    }

    g->syndefault[idx] = mem_strdup(syndefault);
    idx++;
  }

  g->num_syndefault = idx;

  (void) fclose(default_f);
  if (!err_be_quiet()) {
    err_debug("loaded syn default from %s\n", g->default_file);
  }
}

void
delete_syndefault (grammar_t *g)
{
  int i;
  myassert(g);
  if (g->syndefault) {
    for (i = 0; i < g->num_syndefault; i++)
      {
	mem_free(g->syndefault[i]);
      }
    mem_free(g->syndefault);
  }
}
