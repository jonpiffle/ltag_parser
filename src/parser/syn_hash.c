
/* syn_hash - read syntax flat file format into a hash table */

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

static char rcsid[] = "$Id: syn_hash.c,v 1.1 2000/06/06 16:16:01 anoop Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "myassert.h"
#include "hash.h"
#include "mem.h"
#include "err.h"
#include "syn_entry.h"
#include "syn_hash.h"

char syn_buf[SYN_BUFSZ];
char *syn_bufptr;
syn_entrylist_t *syn_entries;

int syn_file_length(FILE *fd)
{
  struct stat stat_buf;
  if(fstat(fileno(fd), &stat_buf) != -1)
    return(stat_buf.st_size);
  return(-1);
}

void *syn_mmapfile(const char *filename, int *n)
{
  FILE *fd = fopen(filename, "r");
  if(!fd) return fd;
  *n = syn_file_length(fd);
  return(mmap(NULL, *n, PROT_READ,
              MAP_PRIVATE, fileno(fd), 0));
}

void
syn_hashentry_delete (void *syn_entry)
{
  syn_entry_delete((syn_entry_t *) syn_entry);
}

hash_t *
syn_hash (const char *synfile)
{
  unsigned linenum = 0;
  hash_t *tbl;
  char *syn_file;
  int syn_len, p, i;
  char *windex, *bufptr;
  syn_entry_t *new, *prev;
  syn_entrylist_t *head, *last;

  err_debug("opening %s for input", synfile);
  syn_file = syn_mmapfile(synfile, &syn_len);

  tbl = hash_new(SYN_NUMBUCKETS);
  myassert(tbl);
  
  bufptr = (char *) mem_alloc(syn_len * sizeof(char));

  for (p = 0, i = 0; p < syn_len; p++)
    {
      bufptr[i++] = syn_file[p];

      if (bufptr[i-1] == '\n') { 
	bufptr[i-1] = 0; 
	i = 0;
	linenum++;
	if ((linenum % NUM_FOR_DOTS) == 0) { err_debug("."); }

	syn_bufptr = &bufptr[0];
	synparse(); 

	if (syn_entries) { 

	  windex = syn_entries->index;
	  prev = (syn_entry_t *) hash_lookup(tbl, windex);

	  if (prev == 0) {
	    new = (syn_entry_t *) mem_alloc(sizeof(syn_entry_t));
	    new->key = mem_strdup(windex);
	    new->syn_entries = syn_entries;
	    new->next = 0;
	    hash_enter(tbl, windex, (void *) new);
	  } else {
	    for (head = last = prev->syn_entries; head; last = head, head = head->next) 
	      {
	      }
	    last->next = syn_entries;
	  }

	}
      }
    }

  mem_free(bufptr);
  err_debug("\n");

#ifdef DEBUG_SYN_HASH
  hash_debug(tbl);
#endif

  return(tbl);
}

/*
#define SYNTAX_FILE    "syntax.flat"
main()
{
  hash_t *hash_tbl;
  hash_tbl = syn_hash(SYNTAX_FILE);
  syn_entry_debug((syn_entry_t *) hash_lookup(hash_tbl, "tell"));
  syn_entry_debug((syn_entry_t *) hash_lookup(hash_tbl, "was"));
  syn_entry_debug((syn_entry_t *) hash_lookup(hash_tbl, "give"));
}
*/
