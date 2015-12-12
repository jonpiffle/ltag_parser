
/* hash - string hash-tables */

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

static char rcsid[] = "$Id: hash.c,v 1.3 2000/07/04 19:23:57 anoop Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myassert.h"
#include "err.h"
#include "mem.h"
#include "hash.h"

static unsigned
nhash (const unsigned char *str, size_t len)
{
    unsigned val = 0;
    size_t i;

    for (i = 0; i < len; ++i) 
      {
	val = ((val << 8) | str[i]) ^ (val >> 24);
      }
    return(val);
}


hash_t *
hash_new (int num_buckets)
{
    hash_t *sh;
    size_t size;

    if (num_buckets <= 0) {
      err_abort("hash_new", 
		"num_buckets must be gt than 0 (not %d)\n",
		num_buckets);
    }

    if (num_buckets & (num_buckets - 1)) {
      err_abort("hash_new",
		"num_buckets must be a power of two (not %d)\n", 
		num_buckets);
    }

    sh = (hash_t *) mem_alloc(sizeof(hash_t));
    sh->num_buckets = num_buckets;
    size = num_buckets * sizeof(hashentry_t *);
    sh->tab = (hashentry_t **) mem_alloc(size);

#ifdef USE_MEMCPY
    memset(sh->tab, 0, size);
#else
    { 
      int i;
      for (i = 0; i < num_buckets; i++)
	{
	  sh->tab[i] = 0;
	}
    }
#endif

    return(sh);
}

void
hash_delete (hash_t *sh, void (*delete) (void *))
{
  int i, sz, k;
  hashentry_t *j, *t;

  myassert(sh);

  sz = sh->num_buckets;
  for (i = 0; i < sz; i++)
    {
      k = 0;
      j = sh->tab[i];
      while (j)
	{
	  t = j;
	  j = j->next;
	  delete(t->value);
	  mem_free(t);
	}
    }
  mem_free(sh->tab);
  mem_free(sh);
}

void
hash_delete_with_key_delete (hash_t *sh, void (*delete) (void *))
{
  int i, sz, k;
  hashentry_t *j, *t;

  myassert(sh);

  sz = sh->num_buckets;
  for (i = 0; i < sz; i++)
    {
      k = 0;
      j = sh->tab[i];
      while (j)
	{
	  t = j;
	  j = j->next;
	  delete(t->value);
	  if (t->key) mem_free(t->key);
	  mem_free(t);
	}
    }
  mem_free(sh->tab);
  mem_free(sh);
}

void
hash_debug (hash_t *sh)
{
  int i, sz;
  hashentry_t *j;

  sz = sh->num_buckets;
  err_debug("hash num_buckets = %d\n", sz);
  for (i = 0; i < sz; i++)
    {
      if (sh->tab[i] == 0) { continue; }
      for (j = sh->tab[i]; j; j = j->next)
	{
	  err_debug("%d, key: %s\n", i, j->key);
	}
    }
}

void
hash_enter (hash_t *sh, char *name, void *value)
{
    hashentry_t *e;
    unsigned i;

    myassert(name);
    myassert(value);

    i = nhash((const unsigned char *) name, strlen(name)) & (sh->num_buckets - 1);
    e = (hashentry_t *) mem_alloc(sizeof(hashentry_t));
    e->next = sh->tab[i];
    sh->tab[i] = e;
    e->key = name;
    e->value = value;
}


void *
hash_lookup (hash_t *sh, const char *name)
{
    hashentry_t *e;
    unsigned i;

    myassert(name);

    i = nhash((const unsigned char *) name, strlen(name)) & (sh->num_buckets - 1);

    for (e = sh->tab[i]; e; e = e->next) {
	if (strcmp(e->key, name) == 0) {
	    return(e->value);
	}
    }
    return(0);
}


void *
hash_nlookup (hash_t *sh, const char *name, size_t len)
{
    hashentry_t *e;
    unsigned i;

    myassert(name);

    i = nhash((const unsigned char *) name, len) & (sh->num_buckets - 1);

    for (e = sh->tab[i]; e; e = e->next) {
	if (strncmp(e->key, name, len) == 0 && e->key[len] == '\0') {
	    return(e->value);
	}
    }
    return(0);
}

void
hash_vdelete (void *v)
{
  if (v) { mem_free(v); }
}
