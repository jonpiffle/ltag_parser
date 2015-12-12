
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

#ifndef _GRAM_PROJECT_H
#define _GRAM_PROJECT_H

#ifdef	__cplusplus
extern "C" {
#endif

/* definitions for project_tags[] */
#define NUMTREES        1
#define NUMFAMILIES     2
#define NUMTREENODES    3
#define NUMLABELS       4
#define NUMSUBSCRIPTS   5
#define WIDTHLABELS     6
#define WIDTHSUBSCRIPTS 7
#define ELEMENTARYTREES 8
#define TREENAMES       9
#define TREENODES       10
#define TREEFEATURES    11
#define TREEFAMILIES    12
#define TREELABELS      13
#define TREESUBSCRIPTS  14
#define WORD_DB         15
#define NUM_EMPTY       16
#define EMPTY_ELTS      17
#define NUM_TOPLABELS   18
#define TOPLABELS       19
#define SYNDEFAULT      20
#define MODELFILE       21
#define ENDTAGS         22 

  /* project_tags_t
     structure which contains the tags used in the project file
     that defines the grammar input files.
     project_tags[0].tag is the seperator used between
     attribute and value.
     project_tags[0].index is the size of the array.
     values defined in the project file are used for the
     grammar_t data structure.  
     */
  static const struct project_tags_s {
    const char *tag;
    int  index;
  } project_tags[ENDTAGS] = {
    { "=",               ENDTAGS },
    { "numTrees",        NUMTREES },
    { "numFamilies",     NUMFAMILIES },
    { "numTreenodes",    NUMTREENODES },
    { "numLabels",       NUMLABELS },
    { "numSubscripts",   NUMSUBSCRIPTS },
    { "widthLabels",     WIDTHLABELS },
    { "widthSubscripts", WIDTHSUBSCRIPTS },
    { "elementaryTrees", ELEMENTARYTREES },
    { "treeNames",       TREENAMES },
    { "treeNodes",       TREENODES },
    { "treeFeatures",    TREEFEATURES },
    { "treeFamilies",    TREEFAMILIES },
    { "treeLabels",      TREELABELS },
    { "treeSubscripts",  TREESUBSCRIPTS },
    { "wordDatabase",    WORD_DB },
    { "numEmpty",        NUM_EMPTY },
    { "emptyElements",   EMPTY_ELTS },
    { "numToplabels",    NUM_TOPLABELS },
    { "topLabels",       TOPLABELS },
    { "defaultEntry",    SYNDEFAULT },
    { "modelFile",       MODELFILE }
  };


#ifdef	__cplusplus
}
#endif

#endif /* _GRAM_PROJECT_H */
