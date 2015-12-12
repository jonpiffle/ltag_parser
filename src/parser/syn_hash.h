
/* syn_hash - read syntax flat file into a hash table */

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

#ifndef SYN_HASH_H
#define SYN_HASH_H

#include <stdio.h>
#include <stdlib.h>
#include "hash.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define SYN_NUMBUCKETS 8192
#define SYNFLAT_BUFLEN 1024
#define NUM_FOR_DOTS   2000

#ifdef  __cplusplus
	   }
#endif

int syn_file_length(FILE *);
void *syn_mmapfile(const char *, int *);
hash_t *syn_hash (const char *);
void syn_hashentry_delete (void *);

#endif
