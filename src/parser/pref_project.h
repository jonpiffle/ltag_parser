
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

#ifndef _PREF_PROJECT_H
#define _PREF_PROJECT_H

#ifdef	__cplusplus
extern "C" {
#endif

/* definitions for pref_tags[] */
#define PREF_PRINT_ALL_PREFS                   1
#define PREF_ERR_QUIET                         2
#define PREF_GRAM_EMPTY_ELT_WIDTH              3
#define PREF_GRAM_ALLOW_FOOT_ADJOIN            4
#define PREF_GRAM_ALLOW_ANCHOR_ADJOIN          5
#define PREF_ALLOW_EPS_TREES                   6
#define PREF_SYN_XTAG_TREENAMES                7
#define PREF_PRUNE_SYN_LOOKUP                  8
#define PREF_PRUNE_FIRST_PASS                  9
#define PREF_LEX_MTX_LOAD                      10
#define PREF_LEX_MTX_INCR                      11
#define PREF_MAX_EPSILON_TREES                 12
#define PREF_LEX_WARN_WHEN_PRUNING             13
#define PREF_ANCHORS_MATCH_SYN_SUBSCRIPT       14
#define PREF_PARSER_CHART_MTX_LOAD             15
#define PREF_PARSER_CHART_MTX_INCR             16
#define PREF_PARSER_PROCESS_STATES_LEN         17
#define PREF_PARSER_STATES_FOUND_LEN           18
#define PREF_PARSER_STATE_HEAP_LEN             19
#define PREF_PARSER_BACKPTR_HEAP_LEN           20
#define PREF_PARSER_ALLOW_EPS_ADJOIN           21
#define PREF_PARSER_PRINT_TREENODES            22
#define PREF_PARSER_PRINT_FEATS                23
#define PREF_PARSER_PRINT_SPANS                24
#define PREF_PARSER_DISALLOW_LEFTAUX_WRAPPING  25
#define PREF_PARSER_DISALLOW_RIGHTAUX_WRAPPING 26
#define PREF_PARSER_MULTIPLE_ADJUNCTIONS       27
#define PREF_PARSER_MAX_START_COUNT            28
#define PREF_PARSER_MAX_PARSE_TIME_ALLOWED     29
#define PREF_MAX_SENT_LEN                      30
#define PREF_MAX_NBEST_INPUT_LEN               31
#define PREF_MAX_NBEST_FIELD_LEN               32
#define PREF_GRAM_MAX_DEFAULT_ENTRIES          33
#define PREF_BUFFER_STDOUT                     34
#define PREF_BUFFER_STDERR                     35
#define ENDTAGS                                36

  /* pref_tags_t
     structure which contains the tags used in the prefs file.
     pref_tags[0].tag is the seperator used between
     attribute and value.
     pref_tags[0].index is the size of the array.
     */
  static const struct pref_tags_s {
    const char *tag;
    int  index;
  } pref_tags[ENDTAGS] = {
    { "=",                                 ENDTAGS                                },
    { "print_all_prefs",                   PREF_PRINT_ALL_PREFS                   },
    { "err_quiet",                         PREF_ERR_QUIET                         },
    { "empty_element_width",               PREF_GRAM_EMPTY_ELT_WIDTH              },
    { "allow_foot_adjoin",                 PREF_GRAM_ALLOW_FOOT_ADJOIN            },
    { "allow_anchor_adjoin",               PREF_GRAM_ALLOW_ANCHOR_ADJOIN          },
    { "allow_eps_trees",                   PREF_ALLOW_EPS_TREES                   },
    { "syn_xtag_treenames",                PREF_SYN_XTAG_TREENAMES                },
    { "prune_syn_lookup",                  PREF_PRUNE_SYN_LOOKUP                  },
    { "prune_first_pass",                  PREF_PRUNE_FIRST_PASS                  },
    { "lex_mtx_load",                      PREF_LEX_MTX_LOAD                      },
    { "lex_mtx_incr",                      PREF_LEX_MTX_INCR                      },
    { "max_epsilon_trees",                 PREF_MAX_EPSILON_TREES                 },
    { "warn_when_pruning",                 PREF_LEX_WARN_WHEN_PRUNING             },
    { "anchors_match_syn_subscript",       PREF_ANCHORS_MATCH_SYN_SUBSCRIPT       },
    { "parser_chart_mtx_load",             PREF_PARSER_CHART_MTX_LOAD             },
    { "parser_chart_mtx_incr",             PREF_PARSER_CHART_MTX_INCR             },
    { "parser_process_states_len",         PREF_PARSER_PROCESS_STATES_LEN         },
    { "parser_states_found_len",           PREF_PARSER_STATES_FOUND_LEN           },
    { "parser_state_heap_len",             PREF_PARSER_STATE_HEAP_LEN             },
    { "parser_backptr_heap_len",           PREF_PARSER_BACKPTR_HEAP_LEN           },
    { "parser_allow_eps_adjoin",           PREF_PARSER_ALLOW_EPS_ADJOIN           },
    { "parser_print_treenodes",            PREF_PARSER_PRINT_TREENODES            },
    { "parser_print_feats",                PREF_PARSER_PRINT_FEATS                },
    { "parser_print_spans",                PREF_PARSER_PRINT_SPANS                },
    { "parser_disallow_leftaux_wrapping",  PREF_PARSER_DISALLOW_LEFTAUX_WRAPPING  },
    { "parser_disallow_rightaux_wrapping", PREF_PARSER_DISALLOW_RIGHTAUX_WRAPPING },
    { "parser_multiple_adjunctions",       PREF_PARSER_MULTIPLE_ADJUNCTIONS       },
    { "parser_max_start_count",            PREF_PARSER_MAX_START_COUNT            },
    { "parser_max_parse_time_allowed",     PREF_PARSER_MAX_PARSE_TIME_ALLOWED     },
    { "max_sentence_length",               PREF_MAX_SENT_LEN                      },
    { "max_nbest_input_len",               PREF_MAX_NBEST_INPUT_LEN               },
    { "max_nbest_field_len",               PREF_MAX_NBEST_FIELD_LEN               },
    { "max_default_entries",               PREF_GRAM_MAX_DEFAULT_ENTRIES          },
    { "buffer_stdout",                     PREF_BUFFER_STDOUT                     },
    { "buffer_stderr",                     PREF_BUFFER_STDERR                     }
  };


#ifdef	__cplusplus
}
#endif

#endif /* _PREF_PROJECT_H */
