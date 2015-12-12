
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

#ifndef HASH_H
#define HASH_H

#ifdef  __cplusplus
extern "C" {
#endif

  typedef struct hashentry_s {
    char *key;
    void *value;
    struct hashentry_s *next;
  } hashentry_t;

  typedef struct hash_s {
    int	num_buckets;
    hashentry_t **tab;
  } hash_t;

#ifdef  __cplusplus
}
#endif

extern hash_t *hash_new (int );
void hash_delete (hash_t *, void (*) (void *));
void hash_delete_with_key_delete (hash_t *, void (*) (void *));
void hash_debug (hash_t *);
extern void hash_enter (hash_t *, char *, void *);
extern void *hash_lookup (hash_t *, const char *);
extern void *hash_nlookup (hash_t *, const char *, size_t);
void hash_vdelete (void *);

#endif /* HASH_H */
