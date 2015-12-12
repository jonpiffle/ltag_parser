
/* pref - handle parser/grammar preferences at runtime */

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

static char rcsid[] = "$Id: pref.c,v 1.5 2000/11/07 21:53:20 anoop Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myassert.h"
#include "err.h"
#include "pref_project.h"
#include "pref_defn.h"
#include "pref.h"

#define NUMBER           "%d\n"
#define FLOATNUM         "%f\n"
#define FILENAME         "%s\n"

void
read_prefs (const char *filename)
{
  FILE *pref_f;
  int tag_index, linenum, fstatus = 0;
  char line[PREF_LINELEN];
  char *attr, *value;
  int be_quiet;

  be_quiet = err_be_quiet();

  if ((pref_f = fopen(filename, "r")) == NULL) {
    err_abort("read_prefs", "could not open prefs file %s\n", filename);
  }
  linenum = 0;
  while ((fstatus = fscanf(pref_f, "%s\n", line)))
    {
      if (fstatus == EOF) break;
      linenum++;
      pref_linesplit(line, &attr, &value);
      if ((tag_index = pref_getindex(attr)) == 0) {
	err_warning("read_prefs", "syntax error in %s line %d\n", filename, linenum);
	continue;
      }
      switch(tag_index) {
      case PREF_PRINT_ALL_PREFS:
	_print_all_prefs = atoi(value);
	break;
      case PREF_ERR_QUIET:
	_err_quiet = atoi(value);
	break;
      case PREF_GRAM_EMPTY_ELT_WIDTH:
	_gram_empty_elt_width = atoi(value);
	break;
      case PREF_GRAM_ALLOW_FOOT_ADJOIN:
	_gram_allow_foot_adjoin = atoi(value);
	break;
      case PREF_GRAM_ALLOW_ANCHOR_ADJOIN:
	_gram_allow_anchor_adjoin = atoi(value);
	break;
      case PREF_ALLOW_EPS_TREES:
	_gram_allow_eps_trees = atoi(value);
	break;
      case PREF_SYN_XTAG_TREENAMES:
	_syn_xtag_treenames = atoi(value);
	break;
      case PREF_PRUNE_SYN_LOOKUP:
	_prune_syn_lookup = atoi(value);
	break;
      case PREF_PRUNE_FIRST_PASS:
	_prune_first_pass = atoi(value);
	break;
      case PREF_LEX_MTX_LOAD:
	_lex_mtx_load = (float)atof(value);
	break;
      case PREF_LEX_MTX_INCR:
	_lex_mtx_incr = atoi(value);
	break;
      case PREF_MAX_EPSILON_TREES:
	_max_epsilon_trees = atoi(value);
	break;
      case PREF_LEX_WARN_WHEN_PRUNING:
	_lex_warn_when_pruning = atoi(value);
	break;
      case PREF_ANCHORS_MATCH_SYN_SUBSCRIPT:
	_anchors_match_syn_subscript = atoi(value);
	break;
      case PREF_PARSER_CHART_MTX_LOAD:
	_hpchart_mtx_load = (float)atof(value);
	break;
      case PREF_PARSER_CHART_MTX_INCR:
	_hpchart_mtx_incr = atoi(value);
	break;
      case PREF_PARSER_PROCESS_STATES_LEN:
	_hp_process_states_len = atoi(value);
	break;
      case PREF_PARSER_STATES_FOUND_LEN:
	_hp_states_found_len = atoi(value);
	break;
      case PREF_PARSER_STATE_HEAP_LEN:
	_hp_state_heap_len = atoi(value);
	break;
      case PREF_PARSER_BACKPTR_HEAP_LEN:
	_hp_backptr_heap_len = atoi(value);
	break;
      case PREF_PARSER_ALLOW_EPS_ADJOIN:
	_hp_allow_eps_adjoin = atoi(value);
	break;
      case PREF_PARSER_PRINT_TREENODES:
	_hp_print_tnodes = atoi(value);
	break;
      case PREF_PARSER_PRINT_FEATS:
	_hp_print_feats = atoi(value);
	break;
      case PREF_PARSER_PRINT_SPANS:
	_hp_print_spans = atoi(value);
	break;
      case PREF_PARSER_DISALLOW_LEFTAUX_WRAPPING:
	_hp_disallow_leftaux_wrapping = atoi(value);
	break;
      case PREF_PARSER_DISALLOW_RIGHTAUX_WRAPPING:
	_hp_disallow_rightaux_wrapping = atoi(value);
	break;
      case PREF_PARSER_MULTIPLE_ADJUNCTIONS:
	_hp_multiple_adjunctions = atoi(value);
	break;
      case PREF_PARSER_MAX_START_COUNT:
	_hp_max_start_count = atoi(value);
	break;
      case PREF_PARSER_MAX_PARSE_TIME_ALLOWED:
	_hp_max_parse_time_allowed = atof(value);
	break;
      case PREF_MAX_SENT_LEN:
	_max_sentlen = atoi(value);
	break;
      case PREF_MAX_NBEST_INPUT_LEN:
	_max_nbest_input_len = atoi(value);
	break;
      case PREF_MAX_NBEST_FIELD_LEN:
	_max_nbest_field_len = atoi(value);
	break;
      case PREF_GRAM_MAX_DEFAULT_ENTRIES:
	_gram_max_default_entries = atoi(value);
	break;
      case PREF_BUFFER_STDOUT:
	_buffer_stdout = atoi(value);
	break;
      case PREF_BUFFER_STDERR:
	_buffer_stderr = atoi(value);
	break;
      default:
	err_warning("read_prefs", "syntax error in %s line %d\n", filename, linenum);
	break;
      }
    }
  (void) fclose(pref_f);

  if (!be_quiet) {
    err_debug("loaded preferences from %s\n", filename);
  }
  if (_print_all_prefs) {
    pref_debug();
  }

  return;
}

int 
pref_getindex (char *tag)
{
  int sz = pref_tags[0].index;
  int i;
  for (i = 1; i < sz; i++)
    {
      if (strcmp(tag, pref_tags[i].tag) == 0) {
	return(pref_tags[i].index);
      }
    }
  return(0); /* syntax error */
}

void
pref_linesplit (char *line, char **attr, char **value)
{
  const char *seperator = pref_tags[0].tag;
  *attr = strtok(line, seperator);
  *value = strtok(0, seperator);
  if ((*attr == NULL) || (*value == NULL)) {
    err_abort("read_prefs", "syntax error. perhaps you have spaces in the file\n");
  }
}

void
pref_debug (void)
{
  int sz = pref_tags[0].index;
  int i;
  err_debug("preferences: begin...\n");
  for (i = 1; i < sz; i++)
    {
      err_debug("%s=", pref_tags[i].tag);
      switch(pref_tags[i].index) {
      case PREF_PRINT_ALL_PREFS:
	err_debug(NUMBER, _print_all_prefs);
	break;
      case PREF_ERR_QUIET:
	err_debug(NUMBER, _err_quiet);
	break;
      case PREF_GRAM_EMPTY_ELT_WIDTH:
	err_debug(NUMBER, _gram_empty_elt_width);
	break;
      case PREF_GRAM_ALLOW_FOOT_ADJOIN:
	err_debug(NUMBER, _gram_allow_foot_adjoin);
	break;
      case PREF_GRAM_ALLOW_ANCHOR_ADJOIN:
	err_debug(NUMBER, _gram_allow_anchor_adjoin);
	break;
      case PREF_ALLOW_EPS_TREES:
	err_debug(NUMBER, _gram_allow_eps_trees);
	break;
      case PREF_SYN_XTAG_TREENAMES:
	err_debug(NUMBER, _syn_xtag_treenames);
	break;
      case PREF_PRUNE_SYN_LOOKUP:
	err_debug(NUMBER, _prune_syn_lookup);
	break;
      case PREF_PRUNE_FIRST_PASS:
	err_debug(NUMBER, _prune_first_pass);
	break;
      case PREF_LEX_MTX_LOAD:
	err_debug(FLOATNUM, _lex_mtx_load);
	break;
      case PREF_LEX_MTX_INCR:
	err_debug(NUMBER, _lex_mtx_incr);
	break;
      case PREF_MAX_EPSILON_TREES:
	err_debug(NUMBER, _max_epsilon_trees);
	break;
      case PREF_LEX_WARN_WHEN_PRUNING:
	err_debug(NUMBER, _lex_warn_when_pruning);
	break;
      case PREF_ANCHORS_MATCH_SYN_SUBSCRIPT:
	err_debug(NUMBER, _anchors_match_syn_subscript);
	break;
      case PREF_PARSER_CHART_MTX_LOAD:
	err_debug(FLOATNUM, _hpchart_mtx_load);
	break;
      case PREF_PARSER_CHART_MTX_INCR:
	err_debug(NUMBER, _hpchart_mtx_incr);
	break;
      case PREF_PARSER_PROCESS_STATES_LEN:
	err_debug(NUMBER, _hp_process_states_len);
	break;
      case PREF_PARSER_STATES_FOUND_LEN:
	err_debug(NUMBER, _hp_states_found_len);
	break;
      case PREF_PARSER_STATE_HEAP_LEN:
	err_debug(NUMBER, _hp_state_heap_len);
	break;
      case PREF_PARSER_BACKPTR_HEAP_LEN:
	err_debug(NUMBER, _hp_backptr_heap_len);
	break;
      case PREF_PARSER_ALLOW_EPS_ADJOIN:
	err_debug(NUMBER, _hp_allow_eps_adjoin);
	break;
      case PREF_PARSER_PRINT_TREENODES:
	err_debug(NUMBER, _hp_print_tnodes);
	break;
      case PREF_PARSER_PRINT_FEATS:
	err_debug(NUMBER, _hp_print_feats);
	break;
      case PREF_PARSER_PRINT_SPANS:
	err_debug(NUMBER, _hp_print_spans);
	break;
      case PREF_PARSER_DISALLOW_LEFTAUX_WRAPPING:
	err_debug(NUMBER, _hp_disallow_leftaux_wrapping);
	break;
      case PREF_PARSER_DISALLOW_RIGHTAUX_WRAPPING:
	err_debug(NUMBER, _hp_disallow_rightaux_wrapping);
	break;
      case PREF_PARSER_MULTIPLE_ADJUNCTIONS:
	err_debug(NUMBER, _hp_multiple_adjunctions);
	break;
      case PREF_PARSER_MAX_START_COUNT:
	err_debug(NUMBER, _hp_max_start_count);
	break;
      case PREF_PARSER_MAX_PARSE_TIME_ALLOWED:
	err_debug(FLOATNUM, _hp_max_parse_time_allowed);
	break;
      case PREF_MAX_SENT_LEN:
	err_debug(NUMBER, _max_sentlen);
	break;
      case PREF_MAX_NBEST_INPUT_LEN:
	err_debug(NUMBER, _max_nbest_input_len);
	break;
      case PREF_MAX_NBEST_FIELD_LEN:
	err_debug(NUMBER, _max_nbest_field_len);
	break;
      case PREF_GRAM_MAX_DEFAULT_ENTRIES:
	err_debug(NUMBER, _gram_max_default_entries);
	break;
      case PREF_BUFFER_STDOUT:
	err_debug(NUMBER, _buffer_stdout);
	break;
      case PREF_BUFFER_STDERR:
	err_debug(NUMBER, _buffer_stderr);
	break;
      default:
	err_warning("pref_debug", "should not have reached here\n");
	break;
      }
    }
  err_debug("preferences: end.\n");
}

#ifdef __PREF_TEST
int
main (int argc, char **argv)
{
  if (argc > 1) {
    read_prefs(argv[1]);
  }
}
#endif /* __PREF_TEST */
