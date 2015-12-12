
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

#ifndef _MEM_H
#define _MEM_H

#include <stdlib.h>
#include <string.h>

#ifdef DEBUG_MEM

#define mem_alloc(s)  _mem_alloc(s,__FILE__,__LINE__)
#define mem_strdup(s) _mem_strdup(s,__FILE__,__LINE__)
#define mem_free(p)   _mem_free(p,__FILE__,__LINE__)

extern void *_mem_alloc (size_t, const char *, int );
char *_mem_strdup (const char *, const char *, int );
extern void _mem_free (void *, const char *, int );

extern void mem_debug_init ();
extern void mem_debug_close ();
extern void mem_report ();

#else

#define mem_alloc(s)  malloc(s)
#define mem_strdup(s) strdup(s)
#define mem_free(p)   free(p)

#endif

#endif /* _MEM_H */
