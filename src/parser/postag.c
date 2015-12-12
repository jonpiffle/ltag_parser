
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

static char rcsid[] = "$Id: postag.c,v 1.2 2000/06/13 21:24:54 anoop Exp $";

#include "postag.h"
#include "err.h"
#include "mem.h"

part_of_speech_t *
postaglist_new (char *postag)
{
  part_of_speech_t *new;
  new = (part_of_speech_t *) mem_alloc(sizeof(part_of_speech_t));
  strcpy(new->postag, postag);
  new->next = 0;
  return(new);
}

part_of_speech_t *
postaglist_add (part_of_speech_t *postaglist, char *postag)
{
  part_of_speech_t *new;
  new = (part_of_speech_t *) mem_alloc(sizeof(part_of_speech_t));
  strcpy(new->postag, postag);
  new->next = postaglist;
  return(new);
}

void
postaglist_debug (part_of_speech_t *postaglist)
{
  part_of_speech_t *i;
  err_debug("postaglist:");
  for (i = postaglist; i; i = i->next)
    {
      err_debug(" %s", i->postag);
    }
  err_debug("\n");
}

void
postaglist_delete (void *postaglist)
{
  part_of_speech_t *i, *j;
  i = (part_of_speech_t *)postaglist;
  while (i)
    {
      j = i;
      i = i->next;
      mem_free(j);
    }
}

