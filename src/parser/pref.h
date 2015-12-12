
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

#ifndef _PREF_H
#define _PREF_H

#define PREF_LINELEN  10000

/* all pref variables */

/* pref */
extern int      _print_all_prefs;

/* buffering */
extern int _buffer_stdout;
extern int _buffer_stderr;

/* err */
extern int      _err_quiet;

/* gram */
extern int      _gram_empty_elt_width;
extern int      _gram_allow_foot_adjoin;
extern int      _gram_allow_anchor_adjoin;
extern int      _gram_allow_eps_trees;
extern int      _gram_max_default_entries;

/* syn_prefs.h */
extern int      _syn_xtag_treenames;

/* lex */
extern int      _prune_syn_lookup;
extern int      _prune_first_pass;
extern float    _lex_mtx_load;
extern unsigned _lex_mtx_incr;
extern unsigned _max_epsilon_trees;
extern int      _lex_warn_when_pruning;
extern int      _anchors_match_syn_subscript;
extern int      _max_sentlen;
extern int      _max_nbest_input_len;
extern int      _max_nbest_field_len;

/* hparse */
extern float    _hpchart_mtx_load;
extern unsigned _hpchart_mtx_incr;
extern unsigned _hp_process_states_len;
extern unsigned _hp_states_found_len;
extern unsigned _hp_state_heap_len;
extern unsigned _hp_backptr_heap_len;
extern int      _hp_allow_eps_adjoin;
extern int      _hp_print_tnodes;
extern int      _hp_print_feats;
extern int      _hp_print_spans;
extern int      _hp_disallow_leftaux_wrapping;
extern int      _hp_disallow_rightaux_wrapping;
extern int      _hp_multiple_adjunctions;
extern int      _hp_max_start_count;
extern double   _hp_max_parse_time_allowed;

/* functions from pref.c */
void read_prefs (const char *);
int pref_getindex (char *);
void pref_linesplit (char *, char **, char **);
void pref_debug (void);

#endif /* _PREF_H */
