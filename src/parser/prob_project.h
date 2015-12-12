
/* prob_project - project file for model params */

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

#ifndef _PROBPROJECT_H
#define _PROBPROJECT_H

#ifdef	__cplusplus
extern "C" {
#endif

/* definitions for prob_tags[] */
#define COUNTS_DB                 1
#define COUNTS_SEPARATOR          2
#define COUNTS_NA_STRING          3
#define COUNTS_WORD_POSTAG_TOTAL  4
#define ATTACH_DIVERSITY_FILE     5
#define TOPLABELS_FILE            6
#define POSDICT_FILE              7
#define UNK_POSDICT_FILE          8
#define COUNT_DIVERSITY_FILE      9
#define NUM_COUNT_DIVERSITY      10
#define DIVERSITY_TYPE_NA        11
#define DIVERSITY_TYPE_A1        12
#define DIVERSITY_TYPE_A2        13
#define DIVERSITY_TYPE_A3        14
#define PROB_ENDTAGS             15

  /* prob_tags_t
     structure which contains the tags used in the project file
     that defines the grammar input files.
     prob_tags[0].tag is the seperator used between
     attribute and value.
     prob_tags[0].index is the size of the array.
     values defined in the project file are used for the
     grammar_t data structure.  
     */
  static const struct prob_tags_s {
    const char *tag;
    int  index;
  } prob_tags[PROB_ENDTAGS] = {
    { "=",                     PROB_ENDTAGS },
    { "countsDB",              COUNTS_DB },
    { "countsSeparator",       COUNTS_SEPARATOR },
    { "countsNAString",        COUNTS_NA_STRING },
    { "countsWordPostagTotal", COUNTS_WORD_POSTAG_TOTAL },
    { "attachDiversity",       ATTACH_DIVERSITY_FILE },
    { "topLabels",             TOPLABELS_FILE },
    { "posDict",               POSDICT_FILE },
    { "unknownPosDict",        UNK_POSDICT_FILE },
    { "countDiversity",        COUNT_DIVERSITY_FILE },
    { "numCountDiversity",     NUM_COUNT_DIVERSITY },
    { "diversityTypeNA",       DIVERSITY_TYPE_NA },
    { "diversityTypeA1",       DIVERSITY_TYPE_A1 },
    { "diversityTypeA2",       DIVERSITY_TYPE_A2 },
    { "diversityTypeA3",       DIVERSITY_TYPE_A3 }
  };


#ifdef	__cplusplus
}
#endif

#endif /* _PROBPROJECT_H */
