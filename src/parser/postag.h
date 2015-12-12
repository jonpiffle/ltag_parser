
/* postag - manage a list of parts of speech for the parser */

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

#ifndef _POSTAG_H
#define _POSTAG_H

#define POSTAG_LEN    20

#ifdef  __cplusplus
extern "C" {
#endif

  typedef struct part_of_speech_s {
    char postag[POSTAG_LEN];
    struct part_of_speech_s *next;
  } part_of_speech_t;

#ifdef  __cplusplus
}
#endif

part_of_speech_t *postaglist_new (char *);
part_of_speech_t *postaglist_add (part_of_speech_t *, char *);
void postaglist_delete (void *);
void postaglist_debug (part_of_speech_t *);

#endif /* _POSTAG_H */
