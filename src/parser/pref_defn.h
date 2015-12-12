
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

#ifndef _PREF_DEFN_H
#define _PREF_DEFN_H

/* prefs about prefs -----------------------------------------------*/

int _print_all_prefs = 0;



/* stdout/stderr buffering prefs -----------------------------------*/
int _buffer_stdout = 1;
int _buffer_stderr = 1;



/* error processing prefs ------------------------------------------*/

/* if the parser is to be less verbose set to nonzero value some
   warnings and error messages will still be printed */
int _err_quiet = 0;




/* grammar processing prefs ----------------------------------------*/

/* max length for empty elements in eps_tbl */
int _gram_empty_elt_width     = 20;

/* permit adjunction at footnodes */
int _gram_allow_foot_adjoin   = 0;

/* permit adjunction on the anchor node */
int _gram_allow_anchor_adjoin = 1;

/* permit non-lexicalized trees to participate in parsing */
int _gram_allow_eps_trees     = 0;

/* max number of syntax default entries allowed */
int _gram_max_default_entries = 50;


/* syntax file prefs -----------------------------------------------*/

/* if the syntax file follows the XTAG convention it will
   have control characters as the first char of the treename
   this variable controls if you want to make these control
   chars compatible with the grammar treenames
*/
int _syn_xtag_treenames = 1;




/* lexicon processing prefs ----------------------------------------*/

/* prune entry unless all terminal symbols or anchors are part of the
   input sentence */
int _prune_syn_lookup  = 1;

/* prune entry if terminal symbols or multiple anchors are not in same
   linear order in the input sentence when compared to their order in
   the elementary tree */
int _prune_first_pass  = 1;

/* params for the sparse matrix that stores treenodes per word */
float _lex_mtx_load      = 0.25;
unsigned _lex_mtx_incr      = 50;

/* max number of unlexicalized trees that are read when initializing
   the grammar from the syntax file */
unsigned _max_epsilon_trees = 20;

/* if pruning away an entry print a warning */
int _lex_warn_when_pruning = 0;

/* when matching the anchor of a tree with an entry in the syntactic
   database, if this flag is set to a nonzero value then the subscript
   of the anchor node is combined with the label before matching the
   entry, otherwise if the flag is set to zero then only the label is
   matched. the default for the xtag syntax db is to merge the label
   and subscript.  */
int _anchors_match_syn_subscript = 1;




/* parser prefs ----------------------------------------------------*/

/* parameters to control the size of allocation of the sparse matrix
   used as a chart by the parsing algorithm */
float    _hpchart_mtx_load = 0.6;
unsigned _hpchart_mtx_incr = 50;

/* sets size of heap used to store states to process during each
   iteration of the parser */
unsigned  _hp_process_states_len = 2048;

/* sets size of heap used to store states found during the completion
   step in the parser */
unsigned _hp_states_found_len = 2048;

/* sets size of heap used to store states in the chart */
unsigned _hp_state_heap_len = 1048;

/* sets size of heap used to store back pointers in the chart */
unsigned _hp_backptr_heap_len = 64;

/* permit adjunction on any node in a subtree whose headcorner is
   epsilon */
int _hp_allow_eps_adjoin = 0;

/* print out the root and foot nodes of each tree while printing the
   derivation trees */
int _hp_print_tnodes = 1;

/* print out lexical feature structure templates while printing the
   derivation trees */
int _hp_print_feats = 1;

/* print out spans of each node in the derivation tree */
int _hp_print_spans = 1;

/* disallow adjunction of a left auxiliary tree onto a right auxiliary
   tree to form a wrapping tree while parsing (for faster parsing time
   with less accuracy set to 1) */
int _hp_disallow_leftaux_wrapping = 0;

/* disallow adjunction of a right auxiliary tree onto a left auxiliary
   tree to form a wrapping tree while parsing (for faster parsing time
   with less accuracy set to 1) */
int _hp_disallow_rightaux_wrapping = 0;

/* allow multiple adjunctions at any node where adjunctions are
   permitted */
int _hp_multiple_adjunctions = 0;

/* stop after finding at least this many parses set to zero if the
   parser should find all parses */
int _hp_max_start_count = 0;

/* maximum time allowed for finding a parse. set to zero if the parser
  should have no time restrictions. although the exact use depends on
  the implementation, in most case, this will correspond to CPU time
  and not stopwatch time */
double _hp_max_parse_time_allowed = 240.00;

/* buffer lengths for processing the input to the parser */
int _max_sentlen = 10000;
int _max_nbest_input_len = 10000;
int _max_nbest_field_len = 2048;



#endif /* PREF_DEFN_H */

