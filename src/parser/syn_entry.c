
/* syn_entry - manage entries from the syntactic database */

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

static char rcsid[] = "$Id: syn_entry.c,v 1.8 2001/07/24 14:04:59 anoop Exp $";

#include <stdio.h>
#include <strings.h>

#include "myassert.h"
#include "mem.h"
#include "err.h"
#include "gram.h"
#include "sent.h"
#include "syn_entry.h"

#ifdef USE_BERKELEY_DB
#include "syn_db.h"
#else
#include "hash.h"
#endif

char syn_buf[SYN_BUFSZ];
char *syn_bufptr;
syn_entrylist_t *syn_entries;

syn_entry_t *
syn_entry_new (const sent_t *s, const grammar_t *g, int prune)
{
  word_t *w;
  syn_entry_t *new, *head, *last;
  char tmp_buf[SYN_BUFSZ];
  int i;

  head = last = 0;
  for (w = sent_head(s); w; w = sent_tail(w))
    {
      if (syn_entry_exists(head, w->word)) { continue; }

      new = (syn_entry_t *) mem_alloc(sizeof(syn_entry_t));
      new->key = mem_strdup(w->word);
      new->next = 0;

#ifdef DEBUG_SYN_ENTRY
      err_debug("getting syn entry for %s\n", w->word);
#endif

#ifdef USE_BERKELEY_DB
      {
	int status;

	syn_buf[0] = '\0';
	status = syn_db(g->dbp, w->word, syn_buf);

	if (status) {

#ifdef DEBUG_SYN_ENTRY
	  err_debug("syn_buf=%s\n", syn_buf); 
#endif
	  syn_bufptr = syn_buf;
	  synparse();
	  new->syn_entries = syn_entries;
	} else {
	  new->syn_entries = 0;
	}
      }
#else
      {
	syn_entry_t *hash_entry;

	hash_entry = (syn_entry_t *)hash_lookup((hash_t *)g->dbp, w->word);
	if (hash_entry) {
	  new->syn_entries = syn_entrylist_copy(hash_entry->syn_entries);
	}
	else {
	  new->syn_entries = 0;
	}
      }
#endif

      if (prune && new->syn_entries) { 
	new->syn_entries = syn_entry_prune(new->syn_entries, s, g); 
	if (!new->syn_entries) { 
	  err_warning("syn_entry_new", "pruned all entries for %s\n", new->key);
	}
      }

      if (!new->syn_entries) {
	err_warning("syn_entry_new", "adding defaults for %s\n", new->key);
	syn_buf[0] = '\0';

	for (i = 0; i < g->num_syndefault; i++)
	  {
	    tmp_buf[0] = '\0';
	    sprintf(tmp_buf, g->syndefault[i], new->key, new->key);
	    strcat(syn_buf, tmp_buf);
	  }

#ifdef DEBUG_SYN_ENTRY
	err_debug("default syn_buf=%s\n", syn_buf); 
#endif

	syn_bufptr = syn_buf;
	synparse();
	new->syn_entries = syn_entries;
      }

      if (!new->syn_entries) {
	err_warning("syn_entry_new", "could not add defaults for %s\n", new->key);
      }

      if (head == 0) { head = new; }
      if (last) { last->next = new; last = new; } 
      else { last = new; }
    }

#ifdef DEBUG_SYN_ENTRY
  syn_entry_debug(head);
#endif

  return(head);
}

void
syn_entry_delete (syn_entry_t *s)
{
  syn_entry_t *i, *j;
  i = s;
  while (i)
    {
      j = i;
      i = i->next;
      mem_free(j->key);
      syn_entrylist_delete(j->syn_entries); 
      mem_free(j);
    }
}

unsigned
syn_entry_length (syn_entry_t *s)
{
  unsigned len;
  syn_entry_t *i;
  for (i = s, len = 0; i ; i = i->next, len++)
    {
    }
  return(len);
}

int
syn_entry_exists (syn_entry_t *s, const char *key)
{
  syn_entry_t *i;
  for (i = s; i ; i = i->next)
    {
      if (strcmp(i->key, key) == 0) { return(1); }
    }
  return(0);
}

syn_entrylist_t *
syn_entry_prune (syn_entrylist_t *s, const sent_t *sent, const grammar_t *gram)
{
  syn_entrylist_t *head, *cur, *prev, *t;
  syn_list_t *i;
  int ok;

  head = prev = cur = s;
  while (cur)
    {
      ok = 1;
      for (i = cur->entries; i; i = i->next)
	{
	  if (gram_is_epsilon(gram, i->value)) { 

#ifdef DEBUG_SYN_ENTRY
	    err_warning("syn_entry_prune", 
			"%s has multi-anchor tree with an empty element\n", 
			s->index); 
#endif

	    continue; 
	  }
	  if (sent_member(sent, i->value) == 0) { ok = 0; }
	}
      if (!ok) {
	t = cur;
	if (prev != cur) { prev->next = cur->next; }
	if (head == cur) { head = prev = cur->next; }

#ifdef DEBUG_SYN_ENTRY
	err_debug("syn_entry_prune: pruning entry "); 
	syn_list_debug(t->entries);
#endif

	cur = cur->next;
	t->next = 0;
	syn_entrylist_delete(t);
      } else {
	prev = cur;
	cur = cur->next;
      }
    }
  return(head);
}

void
syn_entry_debug (syn_entry_t *s)
{
  syn_entry_t *i;
  i = s;
  while (i)
    {
      err_debug("word: %s\n", i->key);
      syn_entrylist_debug(i->syn_entries);
      i = i->next;
    }
}

syn_entrylist_t *
syn_entrylist_new ()
{
  syn_entrylist_t *new;

  new = (syn_entrylist_t *) mem_alloc(sizeof(syn_entrylist_t));
  new->index = 0;
  new->num_entries = 0;
  new->num_trees = 0;
  new->num_families = 0;
  new->num_features = 0;
  new->entries = 0;
  new->parts_of_speech = 0;
  new->trees = 0;
  new->families = 0;
  new->features = 0;
  new->next = 0;
  return(new);
}

void
syn_entrylist_delete (syn_entrylist_t *head)
{
  syn_entrylist_t *i, *j;
  i = head;
  while (i)
    {
      j = i;
      i = i->next;
      mem_free(j->index);
      syn_list_delete(j->entries);
      syn_list_delete(j->parts_of_speech);
      syn_list_delete(j->trees);
      syn_list_delete(j->families);
      syn_list_delete(j->features);
      mem_free(j);
    }
}

unsigned
syn_entrylist_length (syn_entrylist_t *head)
{
  unsigned length;
  syn_entrylist_t *i;
  for (i = head, length = 0; i; i = i->next, length++)
    {
    }
  return(length);
}

syn_entrylist_t *
syn_entrylist_append (syn_entrylist_t *head, syn_entrylist_t *tail)
{
  head->next = tail;
  return(head);
}

void
syn_entrylist_debug (syn_entrylist_t *head)
{
  syn_entrylist_t *i;
  i = head;
  while (i)
    {
      if (i->index) err_debug("index: %s\n", i->index);

      if (i->num_entries) { 
	err_debug("entries: "); 
	syn_list_debug(i->entries); 

	err_debug("pos: "); 
	syn_list_debug(i->parts_of_speech); 
      }

      if (i->num_trees) { 
	err_debug("trees: "); 
	syn_list_debug(i->trees); 
      }

      if (i->num_families) { 
	err_debug("families: "); 
	syn_list_debug(i->families); 
      }

      if (i->num_features) { 
	err_debug("features: "); 
	syn_list_debug(i->features); 
      }

      i = i->next;
    }
}

syn_entrylist_t *
syn_entrylist_copy (syn_entrylist_t *head)
{
  syn_entrylist_t *new = 0;

  if (head == 0) { 
    return(0); 
  } else {
    new = (syn_entrylist_t *) mem_alloc(sizeof(syn_entrylist_t));
    new->index = mem_strdup(head->index);
    new->num_entries = head->num_entries;
    new->num_trees = head->num_trees;
    new->num_families = head->num_families;
    new->num_features = head->num_features;
    new->entries = syn_list_copy(head->entries);
    new->parts_of_speech = syn_list_copy(head->parts_of_speech);
    new->trees = syn_list_copy(head->trees);
    new->families = syn_list_copy(head->families);
    new->features = syn_list_copy(head->features);
    new->next = syn_entrylist_copy(head->next);
    return(new);
  }
}

syn_list_t *
syn_list_new (char *value)
{
  syn_list_t *new;

  new = (syn_list_t *) mem_alloc(sizeof(syn_list_t));
  new->value = value;
  new->next = 0;
  return(new);
}

void
syn_list_delete (syn_list_t *head)
{
  syn_list_t *i, *j;
  i = head;
  while (i)
    {
      j = i;
      i = i->next;
      mem_free(j->value);
      mem_free(j);
    }
}

unsigned
syn_list_length (syn_list_t *head)
{
  unsigned length;
  syn_list_t *i;
  for (i = head, length = 0; i; i = i->next, length++)
    {
    }
  return(length);
}

syn_list_t *
syn_list_append (syn_list_t *head, syn_list_t *tail)
{
  head->next = tail;
  return(head);
}

void
syn_list_debug (syn_list_t *head)
{
  if (!head) { err_debug("\n"); return; }
  err_debug("%s ", head->value);
  syn_list_debug(head->next);
}

syn_list_t *
syn_list_copy (syn_list_t *head)
{
  syn_list_t *new = 0;

  if (head == 0) { 
    return(0); 
  } else {
    new = (syn_list_t *) mem_alloc(sizeof(syn_list_t));
    new->value = mem_strdup(head->value);
    new->next = syn_list_copy(head->next);
    return(new);
  }
}

void
syn_list_split (syn_list_t *head, syn_list_t **s1, syn_list_t **s2)
{
  syn_list_t *i, *j;

  myassert(head);
  myassert(head->next);

  *s1 = i = head;
  *s2 = j = head->next;

  while (i->next)
    {
      i->next = i->next->next;
      i = i->next;
      if (i) { j->next = i->next; }
      else { break; }
      j = j->next;
    }
}

syn_entry_t *
syn_entry_epsilons (const sent_t *s, const grammar_t *g)
{
  int i, sz;
  char *eps;
  syn_entry_t *new, *head, *last;

  sz = gram_num_eps(g);
  head = last = 0;
  for (i = 0; i < sz; i++)
    {
      eps = gram_empty_elt(g,i);
      if (syn_entry_exists(head, eps)) { continue; }
      new = (syn_entry_t *) mem_alloc(sizeof(syn_entry_t));
      new->key = mem_strdup(eps);
      new->syn_entries = 0;
      new->next = 0;

      syn_buf[0] = '\0';

#ifdef USE_BERKELEY_DB
      {
	int status;

	status = syn_db(g->dbp, eps, syn_buf);

	if (status) {

#ifdef DEBUG_SYN_ENTRY
	  err_debug("%s\n", syn_buf); 
#endif

	  syn_bufptr = syn_buf;
	  synparse();
	  new->syn_entries = syn_entries;
	} else {
	  new->syn_entries = 0;
	}
      }
#else
      {
	syn_entry_t *hash_entry;

	hash_entry = (syn_entry_t *)hash_lookup((hash_t *)g->dbp, eps);
	if (hash_entry) {
	  new->syn_entries = syn_entrylist_copy(hash_entry->syn_entries);
	} else {
	  new->syn_entries = 0;
	}
      }	
#endif

      if (head == 0) { head = new; }
      if (last) { last->next = new; last = new; } 
      else { last = new; }
    }
  return(head);
}

#define DEFAULT_POSTAG "-"
#define DEFAULT_LOGPROB -12.00

void
syn_entry_pretty_print (const sent_t *s, const grammar_t *g, syn_entry_t *entries)
{
  char *w;
  int i, sz;
  int id, fsz, j;
  syn_entry_t *windex;
  syn_entrylist_t *synlist;
  syn_list_t *entry, *anchor, *tree, *family;

  /* print header */
  err_print("begin sent=\"");
  sent_print(s);
  err_print("\"\n");

  if (entries == 0) { err_print("end\n"); return; }

  sz = sent_length(s);
  for (i = 0; i < sz; i++)
    {
      w = sent_word_at_index(s, i);
      for (windex = entries; windex; windex = windex->next)
	{
	  for (synlist = windex->syn_entries; synlist; synlist = synlist->next)
	    {
	      entry = synlist->entries;
	      anchor = synlist->parts_of_speech;
	      while (entry)
		{
		  if (anchor == 0) { 
		    err_abort("syn_entry_pretty_print", 
			      "entry and anchor list do not match for: %s", 
			      synlist->index);
		  }

		  if (strcmp(entry->value, w) == 0) {

		    for (tree = synlist->trees; tree; tree = tree->next)
		      {
			err_print("%d %s %s %s %s %lf\n", 
				  i, w, 
				  DEFAULT_POSTAG, 
				  anchor->value, 
				  tree->value, 
				  DEFAULT_LOGPROB);
		      }

		    for (family = synlist->families; family; family = family->next)
		      {
			id = gram_familyindex(g, family->value);
			if (id == -1) {
			  err_warning("syn_entry_pretty_print", 
				      "family %s not found. data may be stale.\n", 
				      family->value);
			  break;
			}
			fsz = gram_familysize(g,id);
			for (j = 0; j < fsz; j++)
			  {
			    err_print("%d %s %s %s %s %lf\n", 
				      i, w, 
				      DEFAULT_POSTAG, 
				      anchor->value, 
				      gram_treename(g, gram_family_tree(g, id, j)),
				      DEFAULT_LOGPROB);
			  }
		      }

		  }
		  entry = entry->next;
		  anchor = anchor->next;
		}
	    }
	}
    }

  /* print trailer */
  err_print("end\n");
}

#ifdef SYN_GET

/* corpus parameters */
#define SPACE    " \t\n"
#define USCORE   "_"
#define LBRACKET "["
#define RBRACKET "]"

int
main (int argc, char **argv)
{
  syn_entry_t *entries;
  sent_t *s;
  grammar_t *g;
  corpus_t *corpus;
  char *sentbuf;
  char *status;
  FILE *stream;

  if (argc != 3) {
    err_abort("usage", "%s <grammar> <preferences>\n", argv[0]);
  }

#ifdef DEBUG_MEM
  mem_debug_init();
#endif

  /* force silence before reading prefs */
  _err_quiet = 1;
  read_prefs(argv[2]);

  /* set up buffering prefs */
  if (_buffer_stdout == 0) {
    setvbuf(stdout, NULL, _IONBF, 0);
  }

  if (_buffer_stderr == 0) {
    setvbuf(stderr, NULL, _IONBF, 0);
  }

  /* force silence after reading prefs */
  _err_quiet = 1;
  g = gram_new(argv[1]);
  corpus = corpus_new(g, SPACE, USCORE, LBRACKET, RBRACKET);
  sentbuf = (char *)mem_alloc(_max_sentlen * sizeof(char));
  stream = stdin;

  if (!err_be_quiet()) {
    err_debug("ready...\n");
  }

  for (;;)
    {
      status = fgets(sentbuf, _max_sentlen, stream);
      if (feof(stream) || (status == 0)) { break; }
      s = sent_new(corpus, sentbuf);
      entries = syn_entry_new(s, g, _prune_syn_lookup);
      if (!entries) { 
	err_message("lex_new", "no entries found for entire sentence\n");
      }
#ifdef DEBUG_SYN_ENTRY
      syn_entry_debug(entries);
#endif
      syn_entry_pretty_print(s, g, entries);
      syn_entry_delete(entries);
      sent_delete(s);
    }

  mem_free(sentbuf);
  corpus_delete(corpus);
  gram_delete(g);
  if (!err_be_quiet) {
    err_debug("done.\n");
  }

#ifdef DEBUG_MEM
  mem_debug_close();
  mem_report();
#endif

  return(0);
}

#endif
