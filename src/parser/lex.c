
/* lex - lexicalized tree management */

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

static char rcsid[] = "$Id: lex.c,v 1.8 2000/11/06 00:49:26 anoop Exp $";

#include <string.h>
#include "myassert.h"
#include "mem.h"
#include "err.h"
#include "sent.h"
#include "gram.h"
#include "syn_entry.h"
#include "spar.h"
#include "lex.h"

lex_t *
lex_new (const sent_t *s, const grammar_t *g)
{
  lex_t *new;
  syn_entry_t *entries;
  unsigned num_trees;

  myassert(s);
  myassert(g);

  entries = syn_entry_new(s, g, _prune_syn_lookup);
  if (!entries) { 
    err_message("lex_new", "no entries found for entire sentence\n");
    return(0);
  }

  num_trees = gram_num_trees(g);

  new = (lex_t *) mem_alloc(sizeof(lex_t));
  new->gram = g;
  new->sent = s;

  new->max_eps = _max_epsilon_trees;
  new->num_eps = 0;
  new->eps_tbl = (epstree_t *) mem_alloc(new->max_eps * sizeof(epstree_t));

  new->lex_mtx = spar_new(sent_length(s), num_trees, 
			  _lex_mtx_load, _lex_mtx_incr);
  lex_lexicalize(new, entries);
  syn_entry_delete(entries);

  /* add trees anchored by empty elements if they are allowed to
     participate in parsing note that empty elements as terminal
     symbols on lexicalized trees will be handled even without this
     step 
  */
  if (_gram_allow_eps_trees) {

    entries = syn_entry_epsilons(s, g);
    if ((!entries) && (gram_num_eps(g))) { 
      err_warning("lex_new", "no epsilon entries found\n");
    } else {

#ifdef DEBUG_SYN_ENTRY
      syn_entry_debug(entries);
#endif

      lex_lexicalize(new, entries);
      syn_entry_delete(entries);
    }
  }

  /* finished adding empty elements */

  if (_prune_first_pass) { lex_prune(new); }

  /* flag active trees in active */
  new->active_tbl = lex_active(new);

#ifdef DEBUG_LEX
  lex_debug(new);
#endif

  return(new);
}


lex_t *
lex_nbest_new (const sent_t *s, const grammar_t *g)
{
  lex_t *new;
  unsigned num_trees;

  myassert(s);
  myassert(g);

  num_trees = gram_num_trees(g);
  new = (lex_t *) mem_alloc(sizeof(lex_t));
  new->gram = g;
  new->sent = s;

  new->max_eps = _max_epsilon_trees;
  new->num_eps = 0;
  new->eps_tbl = 0;

  new->lex_mtx = spar_new(sent_length(s), num_trees, 
			  _lex_mtx_load, _lex_mtx_incr);

  return(new);
}

int
lex_nbest (lex_t *l, int idx, char *word, char *anchor, char *treename, char *postag)
{
  int i, found;
  int tree;
  int treenode;
  char *cur_anchor;
  leafinfo_t *new, *info;
  unsigned sz;
  const grammar_t *g;

  tree = gram_treeindex(l->gram, treename);
  if (tree == NULL_T) {
    err_warning("lex_nbest", 
		"could not find tree %s: data may be stale\n",
		treename);
    return(0);
  }

  g = l->gram;
  sz = gram_num_anchors(g, tree);
  cur_anchor = mem_alloc(sizeof(char) * (g->width_labels + g->width_subscripts + 1));
  for (i = 0, found = 0, treenode = NULL_T; i < sz; i++)
    {
      treenode = gram_anchor_from_index(g, tree, i);
      strcpy(cur_anchor, gram_label(g, treenode));
      strcat(cur_anchor, gram_subscript(g, treenode));
      if (strcmp(cur_anchor, anchor) == 0) {
	found = 1;
	break;
      }
    }
  mem_free(cur_anchor);

  if (!found) { 
    err_warning("lex_nbest", 
		"no appropriate treenode found in %s for anchor %s\n",
		gram_treename(l->gram, tree), anchor);
    return(0);
  }
  myassert(treenode != NULL_T);

  info = spar_get(l->lex_mtx, idx, tree);
  new = leafinfo_new(treenode, 0, lex_postag_new(postag), 0);
  if (info) { new->next = info; }
  spar_put(l->lex_mtx, idx, tree, new);
  return(1);
}

void
lex_delete (lex_t *l)
{
  unsigned i, k;
  spar_t *s;
  int sz;

  if (!l) return;

  s = l->lex_mtx;
  sz = spar_col_size(s);

  for (i = 0; i < sz; i++)
    {
      for (k = spar_row_start(s,i); k < spar_row_size(s,i); k++)
	{
	  leafinfo_delete(spar_value(s,k));
	}
    }
  spar_delete(l->lex_mtx);

  if (l->eps_tbl) { 
    sz = l->num_eps;
    for (i = 0; i < sz; i++)
      {
	mem_free(l->eps_tbl[i].key);
	lex_feature_delete(l->eps_tbl[i].feat);
      }
    mem_free(l->eps_tbl);
  }

  mem_free(l->active_tbl);
  mem_free(l);
}

void
lex_debug (lex_t *l)
{
  if (l == 0) { return; }
  lex_mtx_debug(l);
  lex_eps_debug(l);
  lex_active_debug(l);
}

void
lex_lexicalize (lex_t *l, syn_entry_t *entries)
{
  int id, fsz;
  unsigned i;
  syn_entry_t *e;
  syn_entrylist_t *el;
  syn_list_t *sl;
  const grammar_t *g;

  g = l->gram;

  for (e = entries; e; e = e->next)
    {
      for (el = e->syn_entries; el; el = el->next)
	{

	  if (el->num_trees) {
	    for (sl = el->trees; sl; sl = sl->next)
	      {
		id = gram_treeindex(g, sl->value);
		if (id == -1) {
		  err_warning("lex", 
			      "tree %s not found. data may be stale.\n", 
			      sl->value);
		} else {
		  lex_get_terminals(l, el, id);
		}
	      }

	  }
	  
	  if (el->num_families) {
	    for (sl = el->families; sl; sl = sl->next)
	      {
		id = gram_familyindex(g, sl->value);
		if (id == -1) {
		  err_warning("lex", 
			      "family %s not found. data may be stale.\n", 
			      sl->value);
		} else {
		  fsz = gram_familysize(g,id);
		  for (i = 0; i < fsz; i++)
		    {
		      lex_get_terminals(l, el, gram_family_tree(g, id, i));
		    }
		}
	      }
	  }

	}
    }
}


void
lex_get_terminals (lex_t *l, syn_entrylist_t *el, int tree)
{
  int i, treenode;
  char *key;
  nodelist_t *terminals, *n;
  char **key_tbl;
  unsigned sz;


  /* add the lexical items anchoring the trees */
  sz  = gram_num_anchors(l->gram, tree);

#ifdef DEBUG_LEX
  err_debug("lex_get_terminals: tree=%s, number of anchors = %u\n", 
	    gram_treename(l->gram, tree), 
	    sz);
#endif

  key_tbl = (char **) mem_alloc(sz * sizeof(char *));

  for (i = 0; i < sz; i++)
    {
      treenode = gram_anchor_from_index(l->gram, tree, i);
      key = lex_match(l->gram, el->entries, el->parts_of_speech, treenode);

      if (!key) { 
	err_warning("lex_get_terminals", 
		    "no appropriate treenode found in %s for entry: ",
		    gram_treename(l->gram, tree));
        syn_list_debug(el->entries);
        continue;
      } else {
	key_tbl[i] = mem_strdup(key);
      }
    }

  /* check if the same terminal can lexicalize more than one 
     anchor in the tree
  */
  if (lex_multiple_anchor_check_key(key_tbl, sz)) {

#ifdef DEBUG_LEX
    err_debug("lex_get_terminals: %s has multiple anchors with same key\n",
	      gram_treename(l->gram, tree));
#endif

    lex_update_multiple_anchor (l, el, key_tbl, sz, tree);

  } else {

    for (i = 0; i < sz; i++)
      {
	treenode = gram_anchor_from_index(l->gram, tree, i);
	if (lex_update(l, el, tree, treenode, key_tbl[i]) == 0) {
	  err_warning("lex_get_terminals", 
		      "no appropriate lexical item <%s> found for %s\n", 
		      key_tbl[i], gram_treename(l->gram, tree));
	}
      }
  }

  for (i = 0; i < sz; i++) { mem_free(key_tbl[i]); }
  mem_free(key_tbl);

  /* add the terminal symbols in the tree, if any */
  terminals = (nodelist_t *) gram_tree_map(l->gram, &gram_terminal, 
					   &nodelist_app_wrap, 
					   gram_rootnode(l->gram, tree));

#ifdef DEBUG_LEX
  nodelist_debug(l->gram, terminals);
#endif

  for (n = terminals; n ; n = n->next)
    {
      treenode = n->treenode;
      if (lex_update(l, el, tree, treenode, gram_label(l->gram, treenode)) == 0) {
	if (_prune_first_pass) { 

	  if (_lex_warn_when_pruning) { 
	    err_warning("lex", 
			"no match for terminal %s: pruning tree %s\n", 
			gram_label(l->gram, treenode), 
			gram_treename(l->gram, tree)); 
	  }

	  lex_prune_tree(l, tree); 
	}
      }
    }
  nodelist_delete(terminals);
}

int
lex_update (lex_t *l, syn_entrylist_t *el, int tree, int treenode, const char *key)
{
  leafinfo_t *new, *info;
  int word, idx;
  word_t *w;

  /* add all trees anchored by epsilon into eps_tbl */
  if (gram_is_epsilon(l->gram, key)) {
    w = 0;

    if (l->num_eps >= l->max_eps) {

      err_warning("lex_update", 
		  "too many trees anchored by epsilon, ignoring %s\n", 
		  gram_treename(l->gram, tree));
      err_hint("change value of _max_epsilon_trees in lex preferences\n");

    } else {

      /* before adding an aux tree check if it has a yield
	 this skips any non-lexicalized auxiliary trees that
	 have no terminals or subst nodes
      */
      if (gram_aux_tree_has_yield(l->gram, tree)) {

#ifdef DEBUG_LEX
	err_debug("adding tree %s for epsilon entry %s\n", 
		  gram_treename(l->gram, tree), key);
#endif

	idx = l->num_eps;

	l->eps_tbl[idx].treenode = treenode;
	l->eps_tbl[idx].key      = mem_strdup(key);
	l->eps_tbl[idx].feat     = lex_feature_new(el->features);

	l->num_eps++;
      }
    }

    /* add all trees anchored by lexical items */
  } else {

    w = sent_head(l->sent);
    while ((word = sent_index(w, key)) != -1)
      {
	w = 0;

	info = spar_get(l->lex_mtx, word, tree);

	new = leafinfo_new(treenode, lex_feature_new(el->features), 0, 0);
	if (info) {  new->next = info; }
	spar_put(l->lex_mtx, word, tree, new);

      }
  }

  return(w ? 0 : 1);
}

int
lex_is_eps_anchor (lex_t *l, int treenode)
{
  unsigned i, sz;
  sz = l->num_eps;
  for (i = 0; i < sz; i++)
    {
      if (l->eps_tbl[i].treenode == treenode) {
	return(i);
      }
    }
  return(-1);
}

char *
lex_eps_feature_value (lex_t *l, int i)
{
  return(l->eps_tbl[i].feat);
}

char *
lex_match (const grammar_t *g, syn_list_t *entries, 
	   syn_list_t *parts_of_speech, int treenode)
{
  syn_list_t *e, *p;
  char *tree_pos;
  char *found;

  found = 0;

  tree_pos = lex_merge(gram_label(g, treenode), gram_subscript(g, treenode));

  for (e = entries, p = parts_of_speech; e; e = e->next, p = p->next)
    {
      if (strcmp(p->value, tree_pos) == 0) {
	found = e->value;
	break;
      }
    }
  mem_free(tree_pos);
  return(found);
}

char *
lex_merge (const char *a, const char *b)
{
  char *new;
  if (_anchors_match_syn_subscript) {
    new = (char *) mem_alloc((strlen(a) + strlen(b) + 1) * sizeof(char));
    strcpy(new, a);
    strcat(new, b);
  } else {
    new = (char *) mem_alloc((strlen(a) + 1) * sizeof(char));
    strcpy(new, a);
  }
  return(new);
}

char *
lex_feature_new (syn_list_t *features)
{
  syn_list_t *f;
  char *nf;
  int flen = 0;
  for (f = features; f; f = f->next)
    {
      flen += strlen(f->value) + 1;
    }
  if (flen == 0) { return (0); }
  nf = (char *) mem_alloc((flen + 1) * sizeof(char));
  strcpy(nf,"");
  for (f = features; f; f = f->next)
    {
      strcat(nf,f->value);
      strcat(nf," ");
    }
  return(nf);
}

const char *
lex_feature_value (char *feat)
{
  return(feat ? feat : "");
}

void
lex_feature_delete (char *f)
{
  if (f) { mem_free(f); }
}

char *
lex_postag_new (const char *p)
{
  return(mem_strdup(p));
}

void
lex_postag_delete (char *p)
{
  if (p) { mem_free(p); }
}

leafinfo_t *
leafinfo_new (int treenode, char *features, char *postag, unsigned pruned)
{
  leafinfo_t *new;

  new = (leafinfo_t *) mem_alloc(sizeof(leafinfo_t));
  new->treenode = treenode;
  new->feat     = features;
  new->postag   = postag;
  new->pruned   = pruned;
  new->next     = 0;
  return(new);
}

void
leafinfo_delete (leafinfo_t *info)
{
  leafinfo_t *i, *t;

  i = info;
  while (i)
    {
      t = i;
      i = i->next;
      lex_feature_delete(t->feat);
      lex_postag_delete(t->postag);
      mem_free(t);
    }
}

/* All the following multiple_anchor functions are only for cases
   where a tree has multiple anchors *and* more than one of these
   anchors have the same key in the input string.

   e.g betaPUsPU has Punct1 and Punct2 anchors both of which get the
   key ","
*/

int
lex_multiple_anchor_check_key (char **key_tbl, unsigned sz)
{
  unsigned i, j;

  if (sz < 1) { return(0); }
  for (i = 0; i < sz; i++)
    {
      for (j = i+1; j < sz; j++)
	{
	  if (strcmp(key_tbl[i], key_tbl[j]) == 0) { return(1); }
	}
    }
  return(0);
}

int
lex_multiple_anchor_count (char *key, int idx, char **key_tbl, unsigned sz)
{
  unsigned i, c;

  c = 0;
  if (sz < 1) { return(0); }
  for (i = idx+1; i < sz; i++)
    {
      if (strcmp(key_tbl[i], key) == 0) { c++; }
    }
  return(c);
}

void
lex_update_multiple_anchor (lex_t *l, syn_entrylist_t *el, char **key_tbl, 
			    unsigned sz, int tree)
{
  leafinfo_t *new, *info;
  int word, treenode;
  word_t *head, *w;
  unsigned i, j, count, num_keys, limit;
  int **index_tbl;
  const grammar_t *gram;

  gram = l->gram;
  head = sent_head(l->sent);
  index_tbl = (int **) mem_alloc(sz * sizeof(int *));
  for (i = 0; i < sz; i++)
    {
      count = sent_count(head, key_tbl[i]);
      if (!count) { continue; }

      index_tbl[i] = (int *) mem_alloc(count * sizeof(int));

      w = sent_head(l->sent);
      j = 0;
      while ((word = sent_index(w, key_tbl[i])) != -1)
	{
	  w = 0;
	  myassert(j < count);
	  index_tbl[i][j++] = word;
	}
    }

  for (i = 0; i < sz; i++)
    {
      count = sent_count(head, key_tbl[i]);
      num_keys = lex_multiple_anchor_count(key_tbl[i], i, key_tbl, sz);

      limit = count - num_keys;
      if (limit == 0) { 

#ifdef DEBUG_LEX
	err_debug("no match for key %s: pruning tree %s\n", 
		  key_tbl[i], gram_treename(gram, tree)); 
#endif

	lex_prune_tree(l, tree); 
	break;
      }

      for (j = i; j < limit; j++)
	{
	  word = index_tbl[i][j];
	  treenode = gram_anchor_from_index(gram, tree, i);

#ifdef DEBUG_LEX
	  err_debug("inserting treenode %s%s in tree %s with value %d(%s)\n", 
		    gram_label(gram,treenode), gram_subscript(gram,treenode),
		    gram_treename(gram, tree), word, 
		    sent_word_at_index(l->sent, word));
#endif

	  info = spar_get(l->lex_mtx, word, tree);
	  new = leafinfo_new(treenode, lex_feature_new(el->features), 0, 0);
	  if (info) { new->next = info; }
	  spar_put(l->lex_mtx, word, tree, new);
	}
    }

  for (i = 0; i < sz; i++) { mem_free(index_tbl[i]); }
  mem_free(index_tbl);
}

void
lex_mtx_debug (lex_t *l)
{
  unsigned i, k, sz;
  spar_t *s;
  leafinfo_t *n;
  const grammar_t *gram;
  int treenode;

#ifdef DEBUG_LEX_MTX
  nodelist_t *nl;
#endif

  s = l->lex_mtx;
  gram = l->gram;

  sz  = spar_col_size(s);

  for (i = 0; i < sz; i++)
    {
      for (k = spar_row_start(s,i); k < spar_row_size(s,i); k++)
	{

#ifdef DEBUG_LEX_MTX
	  err_debug("\nlex: Nodes = ");
	  nl = (nodelist_t *) 
	    gram_tree_map(gram, &nodelist_new_wrap, &nodelist_app_wrap, 
			  gram_rootnode(gram, spar_row(s,k)));
	  nodelist_debug(gram, nl);
	  nodelist_delete(nl);
#endif

#ifdef DEBUG_LEX_MTX
	  err_debug("lex: Tree = ");
	  gram_tree_map(gram, &gram_print_node, &gram_null_append,
			gram_rootnode(gram, spar_row(s,k)));
	  err_debug("\n");
#endif

	  for (n = spar_value(s,k); n; n = n->next)
	    {
	      if (n->pruned) { continue; }
	      treenode = n->treenode;
	      err_debug("lex: %d-%s/%s = %s%s(%d)\n", 
			i, 
			sent_word_at_index(l->sent, i), 
			gram_treename(gram, spar_row(s,k)),
			gram_label(gram, treenode), 
			gram_subscript(gram, treenode), 
			treenode);
#ifdef DEBUG_LEX_MTX
	      err_debug("features: %s\n", lex_feature_value(n->feat));
#endif
	    }
	}
    }
}

void
lex_eps_debug (lex_t *l)
{
  unsigned i;
  const grammar_t *gram;
  int sz, treenode, epstree;

  gram = l->gram;
  sz = l->num_eps;
  for (i = 0; i < sz; i++)
    {
      treenode = l->eps_tbl[i].treenode;
      epstree = gram_tree_from_anchor(gram, l->eps_tbl[i].treenode);
      err_debug("lex_eps: %d %s/%s = %s%s(%d), %s\n", 
		gram_aux_tree_has_yield(gram, epstree), 
		l->eps_tbl[i].key, 
		gram_treename(gram, epstree),
		gram_label(gram, treenode), 
		gram_subscript(gram, treenode), 
		treenode, 
		lex_feature_value(l->eps_tbl[i].feat));
    }
}

