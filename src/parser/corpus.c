
/* corpus - set parameters for all sentences in a corpus */

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

static char rcsid[] = "$Id: corpus.c,v 1.1 2000/06/06 16:03:10 anoop Exp $";

#include "myassert.h"
#include "mem.h"
#include "corpus.h"

corpus_t *
corpus_new (grammar_t *g, const char *wd, const char *pd, const char *ncb, const char *nce)
{
  corpus_t *new;
  new = (corpus_t *) mem_alloc(sizeof(corpus_t));
  new->gram = g;
  new->word_delim = mem_strdup(wd);
  new->pos_delim = mem_strdup(pd);
  new->noun_chunk_begin = mem_strdup(ncb);
  new->noun_chunk_end = mem_strdup(nce);
  return(new);
}

void
corpus_delete (corpus_t *c)
{
  myassert(c);
  if (c->word_delim) mem_free(c->word_delim);
  if (c->pos_delim) mem_free(c->pos_delim);
  if (c->noun_chunk_begin) mem_free(c->noun_chunk_begin);
  if (c->noun_chunk_end) mem_free(c->noun_chunk_end);
  mem_free(c);
}
