/* syntax.y      -*- C -*- */

%{

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

static char rcsid[] = "$Id: syn.y,v 1.1 2000/06/06 16:14:39 anoop Exp $";

#include <string.h>
#include <assert.h>
#include "mem.h"
#include "err.h"
#include "syn_entry.h"

#define ALPHA     "alpha"
#define ALPHA_CH  ''
#define BETA      "beta"
#define BETA_CH   ''

extern syn_entrylist_t *syn_entries;
syn_entrylist_t *syn_cur;

%}

%union {
  char *string;
  syn_list_t *syn_list;
  syn_entrylist_t *syn_entry;
}

%token INDEX ENTRY POS TREES FAMILY FEATURES
%token <string> TOKEN

%type <syn_list> entry_list entry tree_list family_list feature_list
%type <syn_entry> syntax_entry syntax_file
%start syntax_file

%%

syntax_file :	syntax_entry syntax_file 
                {
		  syn_entrylist_append($1, $2);
		  $$ = $1;
		  syn_entries = $1;
                }
	    |	/* no entries */
                {
		  $$ = 0;
		  syn_entries = 0;
                }
	    ;

syntax_entry :	INDEX TOKEN 
		{
		  syn_cur = syn_entrylist_new();
		  syn_cur->index = $2;
		}
                entry_list 
		{
		  syn_list_t *e, *p;
		  unsigned length = syn_list_length($4);
		  e = p = 0;
		  if (!length) err_abort("syn", "no entry found for %s\n", $2);
		  if (length % 2) err_abort("syn", "mismatch in entry and POS for %s\n", $2);
		  syn_list_split($4, &e, &p);
		  syn_cur->num_entries = length / 2;
		  syn_cur->entries = e;
		  syn_cur->parts_of_speech = p;
		}
		tree_or_family 
		features 
		{
		  $$ = syn_cur;
		}
	     ;

entry_list :	entry entry_list
		{
		  $$ = syn_list_append($1, $2);
		}
	   |	/* no (more) entries */
                {
		  $$ = 0;
		}
	   ;

entry :		ENTRY TOKEN 
		{
		  $$ = syn_list_new($2);
		}
      |		POS TOKEN 
		{
		  $$ = syn_list_new($2);
		}
      ;

tree_or_family : TREES tree_list
		{
		  syn_cur->num_trees = syn_list_length($2);
		  syn_cur->trees = $2;
		}
	       | FAMILY family_list
		{
		  syn_cur->num_families = syn_list_length($2);
		  syn_cur->families = $2;
		}
	       | /* no trees or families */
	       ;


tree_list : TOKEN tree_list
	    {
	      char *new;
	      new = pretty_treename($1);
	      mem_free($1);
	      $$ = syn_list_append(syn_list_new(new), $2);
            }
          | /* no (more) trees */
	    {
	      $$ = 0;
            }
          ;

family_list : TOKEN family_list
 	      {
		$$ = syn_list_append(syn_list_new($1), $2);
              }
            | /* no (more) families */
	      {
		$$ = 0;
              }
	    ;

features :  FEATURES feature_list
	    {
	      syn_cur->num_features = syn_list_length($2);
	      syn_cur->features = $2;
	    }
         |  /* no features */
	    {
	      syn_cur->num_features = 0;
	      syn_cur->features = 0;
	    }
         ;

feature_list : TOKEN feature_list
	       {
		$$ = syn_list_append(syn_list_new($1), $2);
	       }
         |     /* no (more) features */
	       {
		$$ = 0;
	       }
	 ;

%%

void
synerror(const char *mesg)
{
  err_warning("synerror", "error in parsing syntax db output: %s\n", mesg);
}

char *
pretty_treename (const char *treename)
{
  char *new;
  assert(treename);
  if (_syn_xtag_treenames) {
    switch(treename[0]) {
    case ALPHA_CH:
      new = (char *) mem_alloc((strlen(treename)+strlen(ALPHA))*sizeof(char));
      strcpy(new, ALPHA);
      strcat(new, treename+1);
      break;
    case BETA_CH:
      new = (char *) mem_alloc((strlen(treename)+strlen(BETA))*sizeof(char));
      strcpy(new, BETA);
      strcat(new, treename+1);
      break;
    default:
      err_warning("syn", "tree %s doesn't follow XTAG name convention\n", treename);
      new = (char *) mem_alloc((strlen(treename)+1)*sizeof(char));
      strcpy(new,treename);
      break;
    }
  } else {
    new = (char *) mem_alloc((strlen(treename)+1)*sizeof(char));
    strcpy(new,treename);
  }
  return(new);
}
