
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

#ifndef _SENT_H
#define _SENT_H

#include "corpus.h"

#ifdef  __cplusplus
extern "C" {
#endif

  typedef struct word_s {
    char *word;
    struct word_s *tail;
  } word_t;

  typedef struct sent_s {
    corpus_t *corpus;
    word_t *head;
    unsigned length;
  } sent_t;

#ifdef  __cplusplus
}
#endif

extern sent_t *sent_new (const corpus_t *, const char *);
extern void sent_delete (sent_t *);
extern unsigned sent_length (const sent_t *);
extern void sent_debug (sent_t *);
extern void sent_print (const sent_t *);
extern word_t *sent_head (const sent_t *);
extern word_t *sent_tail (const word_t *);
extern word_t *sent_member (const sent_t *, const char *);
extern char *sent_word_at_index (const sent_t *, const int );
extern int sent_index (word_t *, const char *);
int sent_count (word_t *, const char *);
extern void call_sent(sent_t *s);

void sent_delete_words (word_t *);
char *sent_split (char *, const char *);

#endif /* _SENT_H */
