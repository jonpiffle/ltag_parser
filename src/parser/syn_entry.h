
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

#ifndef _SYN_ENTRY_H
#define _SYN_ENTRY_H

#include "sent.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define SYN_BUFSZ 524288

  typedef struct syn_list_s {
    char *value;
    struct syn_list_s *next;
  } syn_list_t;

  typedef struct syn_entrylist_s {
    char *index;
    unsigned num_entries;
    unsigned num_trees;
    unsigned num_families;
    unsigned num_features;
    syn_list_t *entries;
    syn_list_t *parts_of_speech;
    syn_list_t *trees;
    syn_list_t *families;
    syn_list_t *features;
    struct syn_entrylist_s *next;
  } syn_entrylist_t;

  typedef struct syn_entry_s {
    char *key;
    syn_entrylist_t *syn_entries;
    struct syn_entry_s *next;
  } syn_entry_t;

#ifdef  __cplusplus
	   }
#endif

extern syn_entry_t *syn_entry_new    (const sent_t *, const grammar_t *, int);
extern void         syn_entry_delete (syn_entry_t *);
extern unsigned     syn_entry_length (syn_entry_t *);
extern void         syn_entry_debug  (syn_entry_t *);
extern int          syn_entry_exists (syn_entry_t *, const char *);

extern syn_entry_t *syn_entry_epsilons (const sent_t *, const grammar_t *);

extern syn_entrylist_t *syn_entrylist_new    ();
extern void             syn_entrylist_delete (syn_entrylist_t *);
extern unsigned         syn_entrylist_length (syn_entrylist_t *);
extern syn_entrylist_t *syn_entrylist_append (syn_entrylist_t *, syn_entrylist_t *);
extern void             syn_entrylist_debug  (syn_entrylist_t *);
extern syn_entrylist_t *syn_entrylist_copy   (syn_entrylist_t *);
extern syn_entrylist_t *syn_entry_prune      (syn_entrylist_t *, const sent_t *, const grammar_t *);

extern syn_list_t *syn_list_new    (char *);
extern void        syn_list_delete (syn_list_t *);
extern unsigned    syn_list_length (syn_list_t *);
extern syn_list_t *syn_list_append (syn_list_t *, syn_list_t *);
extern void        syn_list_debug  (syn_list_t *);
syn_list_t        *syn_list_copy (syn_list_t *);
extern void        syn_list_split  (syn_list_t *, syn_list_t **, syn_list_t **);

extern int  syn_yyinput (char *, int );
extern int  synlex      ();
extern int  synparse    ();
extern void synerror    (const char *);

extern char *pretty_treename (const char *);

#endif /* _SYN_ENTRY_H */
