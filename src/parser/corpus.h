
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

#ifndef _CORPUS_H
#define _CORPUS_H

#include "gram.h"

#ifdef  __cplusplus
extern "C" {
#endif

  typedef struct corpus_s {
    grammar_t *gram;
    char *word_delim;
    char *pos_delim;
    char *noun_chunk_begin;
    char *noun_chunk_end;
  } corpus_t;

#ifdef  __cplusplus
}
#endif

extern corpus_t *corpus_new (grammar_t *, const char *, const char *, const char *, const char *);
extern void corpus_delete (corpus_t *);

#endif /* _CORPUS_H */
