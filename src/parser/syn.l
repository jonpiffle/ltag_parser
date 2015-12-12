/* syntax.l     -*- C -*-  */

%option noyywrap

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

static char rcsid[] = "$Id: syn.l,v 1.1 2000/06/06 16:14:26 anoop Exp $";

#include <string.h>

#define yylval synlval

#include "common.h"
#include "mem.h"
#include "syn_entry.h"
#include "syn.tab.h"

#undef YY_INPUT
#define YY_INPUT(b, r, ms) (r = syn_yyinput(b, ms))
#define YY_NO_UNPUT

extern char syn_buf[];
extern char *syn_bufptr;

%}

token [^ \t\n<>]+
spc   [ \t]+
eol   \n
%%

"<<INDEX>>" return INDEX;

"<<ENTRY>>" return ENTRY;

"<<POS>>" return POS;

"<<TREES>>" return TREES;

"<<FAMILY>>" return FAMILY;

"<<FEATURES>>" return FEATURES;

{token} { synlval.string = mem_strdup(syntext); return TOKEN; }

{spc} ;

{eol} ;

. return(syntext[0]);

%%

int
syn_yyinput (char *buf, int max_size)
{
  int n = MIN((unsigned) max_size, strlen(syn_bufptr));

  if (n > 0) {
    memcpy(buf, syn_bufptr, n);
    syn_bufptr += n;
  }
  return(n);
}

