
/* sent - sentence management */

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

static char rcsid[] = "$Id: sent.c,v 1.1 2000/06/06 16:13:52 anoop Exp $";

#include <string.h>
#include "myassert.h"
#include "mem.h"
#include "err.h"
#include "corpus.h"
#include "sent.h"

sent_t *
sent_new (const corpus_t *corpus, const char *sentence)
{
  sent_t *new;
  word_t *head, *last, *current;
  const char *sep;
  char *s, *word, *copy;
  unsigned length;

  myassert(corpus);
  myassert(sentence);

  copy = mem_strdup(sentence);
  sep = corpus->word_delim;
  head = last = 0;

  for (s = copy, length = 0; (word = sent_split(s, sep)); s = 0, length++)
  {
    current = (word_t *) mem_alloc(sizeof(word_t));
    if (head == 0) head = current;
    current->word = mem_strdup(word);
    current->tail = 0;
    if (last) { last->tail = current; last = current; }
    else { last = current; }
  }
  mem_free(copy);
  if (head == 0) return(0);
  myassert(length);
  new = (sent_t *) mem_alloc(sizeof(sent_t));
  new->head = head;
  new->length = length;

  if (!err_be_quiet()) { sent_debug(new); }
  return(new);
}

void
sent_delete (sent_t *s)
{
  myassert(s);
  sent_delete_words(s->head);
  mem_free(s);
}

void
sent_delete_words (word_t *w)
{
  if (!w) return;
  sent_delete_words(w->tail);
  mem_free(w->word);
  mem_free(w);
}

char *
sent_split (char *sentence, const char *sep)
{
  return(strtok(sentence, sep));
}

unsigned 
sent_length (const sent_t *s)
{
  myassert(s);
  return(s->length);
}

void
sent_debug (sent_t *s)
{
  word_t *w;
  if (s == 0) { return; }
  err_debug("sent: %d: ", sent_length(s));
  for (w = s->head; w ; w = w->tail)
    {
      err_debug("%s ", w->word);
    }
  err_debug("\n");
}

void
sent_print (const sent_t *s)
{
  word_t *w;
  for (w = s->head; w ; w = w->tail)
    {
      if (w->tail == 0) { break; }
      err_print("%s ", w->word);
    }
  err_print("%s", w->word);
}

word_t *
sent_head (const sent_t *s)
{
  return(s->head);
}

word_t *
sent_tail (const word_t *w)
{
  if (w == 0) return(0);
  return(w->tail);
}

word_t *
sent_member (const sent_t *s, const char *key)
{
  word_t *w;
  for (w = s->head; w ; w = w->tail)
    {
      if (strcmp(w->word, key) == 0) return(w);
    }
  return(0);
}

char *
sent_word_at_index (const sent_t *s, const int windex)
{
  int i;
  word_t *w;
  for (w = s->head, i = 0; w ; w = w->tail, i++)
    {
      if (i == windex) return(w->word);
    }
  return(0);
}

int
sent_index (word_t *head, const char *key)
{
  static word_t *w;
  static int windex;
  word_t *i;

  if (head) { w = head; windex = 0; }
  if (!w) { return(-1); }
  for (i = w; i ; i = i->tail)
    {
      windex++;
      if (strcmp(i->word, key) == 0) {
	w = i->tail;
	return(windex-1);
      }
    }
  w = 0;
  return(-1);
}

int
sent_count (word_t *head, const char *key)
{
  int count;
  word_t *i;

  count = 0;
  for (i = head; i ; i = i->tail)
    {
      if (strcmp(i->word, key) == 0) {
	count++;
      }
    }
  return(count);
}

void
call_sent(sent_t *s)
{ 
  word_t *w;
  int windex;
  for (w = s->head; w; w = w->tail)
    {
      err_debug("%s: %d", w->word, sent_index(s->head, w->word));
      while ((windex = sent_index(0, w->word)) != -1)
        {
          err_debug(" %d", windex);
        }
      err_debug("\n");
    }
}

