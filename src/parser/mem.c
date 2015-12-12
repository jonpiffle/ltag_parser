
/* mem - memory management */

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

static char rcsid[] = "$Id: mem.c,v 1.1 2000/06/06 16:12:44 anoop Exp $";

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "myassert.h"
#include "mem.h"
#include "err.h"

#ifdef DEBUG_MEM

/* globals */
static long mem_numallocs = 0;
static int mem_debug = 0;
static char alloc_file[] = "mem.allocs";
static char free_file[] = "mem.frees";
static FILE *alloc_f, *free_f;

void
mem_debug_init ()
{
  mem_debug = 1;
  if ((alloc_f = fopen(alloc_file, "w")) == NULL) {
    err_warning("mem_debug_init: could not open file %s\n", alloc_file);
    mem_debug = 0;
  }
  if ((free_f = fopen(free_file, "w")) == NULL) {
    (void) fclose(alloc_f);
    err_warning("mem_debug_init: could not open file %s\n", free_file);
    mem_debug = 0;
  }
}

void
mem_debug_close ()
{
  if (mem_debug) {
    (void) fclose(alloc_f);
    (void) fclose(free_f);
  }
  mem_debug = 0;
}

void * 
_mem_alloc (size_t sz, const char *f, int l)
{
  register void *new;
  new = malloc(sz);
  if (mem_debug) {
    (void) fprintf(alloc_f, "%p %u %s %d\n", new, sz, f, l);
  }
  myassert(new);
  mem_numallocs++;
  return(new);
}

char * 
_mem_strdup (const char *s, const char *f, int l)
{
  register char *new;
  new = strdup(s);
  if (mem_debug) {
    (void) fprintf(alloc_f, "%p %u %s %d\n", new, strlen(s)+1, f, l);
  }
  myassert(new);
  mem_numallocs++;
  return(new);
}

void 
_mem_free (void *p, const char *f, int l)
{
  if (mem_debug) {
    (void) fprintf(free_f, "%p %s %d\n", p, f, l);
  }
  myassert(p);
  free(p);
  mem_numallocs--;
}

void
mem_report ()
{
  (void) fprintf(stderr, "memory blocks active = %ld\n", mem_numallocs);
}

#endif
