
## Makefile for the lem parser
##
## Copyright (c) 2000 Anoop Sarkar
##
## This source code is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License 
## as published by the Free Software Foundation; either version 2 of 
## the License, or (at your option) any later version. 
## 
## The file COPYING in the current directory has a copy of the GPL. 
##
## This code comes WITHOUT ANY WARRANTY; without even the implied
## warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##

LEMHOME=/Users/piffle/Desktop/xtag_resources/lem-0.14.0
BERKELEY_DB=/usr/local/BerkeleyDB.3.1/
CC=/usr/bin/gcc

# Tested with the following compilers (gcc, bison, flex)
# GCC_VERSION:2.7.2
# GCC_VERSION:2.8.1
# EGCS_VERSION:1.1b

# Conditional compilation flags
# -DUSE_MEMCPY  = machine has memcpy
# -DUSE_BERKELEY_DB = if the Berkeley db package ver 2.x is installed
# -DUSE_ASSERT = adds sanity checks using assert stmts
# -DUSE_PROB = runs parser as a statistical parser
# -DUSE_PARSER_HEURISTICS = uses heuristic methods to limit parse time
# -DALLOW_EPS_TREES = allow the use of non-lexicalized elementary trees

SRC_DEFS=-DUSE_BERKELEY_DB
#SRC_DEFS=-DUSE_MEMCPY -DUSE_BERKELEY_DB
#SRC_DEFS=-DUSE_ASSERT -DUSE_BERKELEY_DB
#SRC_DEFS=-DUSE_ASSERT -DUSE_MEMCPY -DUSE_BERKELEY_DB -DUSE_NBEST_INPUT
#SRC_DEFS=-DUSE_ASSERT -DUSE_MEMCPY -DUSE_BERKELEY_DB -DUSE_PROB -DUSE_NBEST_INPUT -DHPARSE_USE_HEAP_EXTRACT

DEBUG=
#-DDEBUG_MEM
#-DDEBUG_SPAR
#-DDEBUG_PROJECT
#-DDEBUG_LABELS
#-DDEBUG_SUBSCRIPTS
#-DDEBUG_TREENODES
#-DDEBUG_TREES
#-DDEBUG_TREENAMES
#-DDEBUG_TREEFEATURES
#-DDEBUG_TREEINFO
#-DDEBUG_FAMILIES
#-DDEBUG_EPSILONS
#-DDEBUG_TOPLABELS
#-DDEBUG_ENV
#-DDEBUG_SYN_ENTRY
#-DDEBUG_SYN_DB
#-DDEBUG_SYN_HASH
#-DDEBUG_LEX
#-DDEBUG_LEX_MTX
#-DDEBUG_BIN
#-DDEBUG_HAND
#-DDEBUG_IA
#-DDEBUG_HPARSE
#-DDEBUG_HPARSE_ADD
#-DDEBUG_HPARSE_CHART
#-DDEBUG_HPARSE_PROB
#-DDEBUG_COUNTS_DB
#-DDEBUG_PROB

# uncomment if using Berkeley db ver 2.x
# comment otherwise
BERKELEY_DB_HOME=$(BERKELEY_DB)
DBINCLUDE=-I$(BERKELEY_DB_HOME)/include
CONTRIBOBJ=
SYNOBJ=syn_db.o
SYNFLAGS=-L$(BERKELEY_DB_HOME)/lib
SYNLIBS=-ldb

#DB BERKELEY_DB_HOME=$(BERKELEY_DB)
#DB DBINCLUDE=-I$(BERKELEY_DB_HOME)/include
#DB CONTRIBOBJ=
#DB SYNOBJ=syn_db.o
#DB SYNFLAGS=-L$(BERKELEY_DB_HOME)/lib
#DB SYNLIBS=-ldb

# Uncomment if not using Berkeley db
# comment otherwise
#HASH DBINCLUDE=
#HASH CONTRIBOBJ=
#HASH SYNOBJ=syn_hash.o
#HASH SYNFLAGS=
#HASH SYNLIBS=

#CC=gcc-2.8.1
#CC=/pkg/egcs-1.1b/bin/gcc
# for linc/unagi add -L/pkg/lib to $(CFLAGS) for using egcs on unagi/linc
GCC_FLAGS=-O -Wall -Winline -Wshadow -Wwrite-strings -Wcast-qual -Wcast-align $(DEBUG) $(SRC_DEFS)
#GCC_FLAGS=-pedantic -Wall -Wshadow -Wwrite-strings -Wcast-qual -Wcast-align $(DEBUG) $(SRC_DEFS)
OPT_FLAGS=-O3 -fomit-frame-pointer -finline-functions -ffast-math -funroll-loops $(SRC_DEFS)
GDB_FLAGS=-g $(GCC_FLAGS)
PROF_FLAGS=-g -pg $(GCC_FLAGS)
#CFLAGS=$(GCC_FLAGS)
CFLAGS=$(OPT_FLAGS)
#CFLAGS=$(PROF_FLAGS)
#CFLAGS=$(GDB_FLAGS)

SYNENTRY_OBJ=err.o \
mem.o \
pref.o \
corpus.o \
sent.o \
gram.o gram_slurp.o gram_trees.o gram_treemap.o gram_nodelist.o gram_treeinfo.o \
syn.tab.o syn.lex.o \
hash.o

OBJ=err.o \
mem.o \
pref.o \
corpus.o \
sent.o \
gram.o gram_slurp.o gram_trees.o gram_treemap.o gram_nodelist.o gram_treeinfo.o \
bin.o \
syn.tab.o syn.lex.o syn_entry.o \
spar.o \
hash.o \
lex.o lex_prune.o lex_active.o \
ia.o heap.o \
hparse_hand.o \
hparse_add.o hparse_utils.o hparse_state.o \
hparse_word.o hparse_backptr.o \
hparse.o hparse_prob.o hparse_deriv.o \
postag.o prob_counts_db.o prob.o \
parser.o

# Some notes on the use of lex and yacc:
# It is important that flex is used instead of the other 
# varieties of Lex such as POSIX Lex, Pclex, ATT Lex or MKS Lex.
# If you cannot find flex on your platform then you will
# have to modify syn.l so that yylex() accepts input from
# a string. The current method used to accomplish this
# is specific to flex. 
# Also, use bison if possible rather than yacc although
# any version of yacc should work fine.

FLEX=flex
FLEX_FLAGS=-Psyn -t
FLEX_LIBS=-ll

YACC=yacc
YACC_FLAGS=-d -bsyn -psyn
YACC_LIBS=-ly

BISON=bison
BISON_FLAGS=-d -psyn
BISON_LIBS=

L=$(FLEX)
LFLAGS=$(FLEX_FLAGS)
Y=$(BISON)
YFLAGS=$(BISON_FLAGS)
YLIBS=$(BISON_LIBS) $(FLEX_LIBS)

LIBS=-lm

RM=/bin/rm -f

all: parser.bin syn_get.bin remove_hparse_hand nbest_parser.bin

parser.bin: $(OBJ) $(SYNOBJ) main.c
	$(CC) $(CFLAGS) -c hparse_hand.c
	$(CC) $(CFLAGS) $(SYNFLAGS) -o parser.bin $(OBJ) $(SYNOBJ) $(CONTRIBOBJ) main.c $(LIBS) $(SYNLIBS) $(YLIBS)

nbest_parser.bin: $(OBJ) $(SYNOBJ) main.c
	$(CC) $(CFLAGS) -DUSE_NBEST_INPUT -c hparse_hand.c
	$(CC) $(CFLAGS) $(SYNFLAGS) -DUSE_NBEST_INPUT -o nbest_parser.bin $(OBJ) $(SYNOBJ) $(CONTRIBOBJ) main.c $(LIBS) $(SYNLIBS) $(YLIBS)

syn_get.bin: $(SYNENTRY_OBJ) $(SYNOBJ) syn_entry.c
	$(CC) $(CFLAGS) $(SYNFLAGS) -DSYN_GET -o syn_get.bin $(SYNENTRY_OBJ) $(SYNOBJ) $(CONTRIBOBJ) syn_entry.c $(LIBS) $(SYNLIBS) $(YLIBS)

remove_hparse_hand:
	$(RM) hparse_hand.o

install:
	/bin/cp $(LEMHOME)/src/parser/parser.bin $(LEMHOME)/bin/
	/bin/cp $(LEMHOME)/src/parser/nbest_parser.bin $(LEMHOME)/bin/
	/bin/cp $(LEMHOME)/src/parser/syn_get.bin $(LEMHOME)/bin/

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c

postag.o: postag.c postag.h
	$(CC) $(CFLAGS) -c postag.c

prob.o: prob.c prob.h
	$(CC) $(CFLAGS) -c prob.c

hparse_hand.o: hparse_hand.c hparse_hand.h
	$(CC) $(CFLAGS) -c hparse_hand.c

hparse.o: hparse.c hparse.h hparse_hand.h
	$(CC) $(CFLAGS) -c hparse.c

hparse_utils.o: hparse_utils.c hparse.h hparse_hand.h
	$(CC) $(CFLAGS) -c hparse_utils.c

hparse_add.o: hparse_add.c hparse.h hparse_hand.h
	$(CC) $(CFLAGS) -c hparse_add.c

hparse_state.o: hparse_state.c hparse.h hparse_hand.h
	$(CC) $(CFLAGS) -c hparse_state.c

hparse_word.o: hparse_word.c hparse.h hparse_hand.h
	$(CC) $(CFLAGS) -c hparse_word.c

hparse_backptr.o: hparse_backptr.c hparse.h
	$(CC) $(CFLAGS) -c hparse_backptr.c

hparse_prob.o: hparse_prob.c hparse.h hparse_hand.h
	$(CC) $(CFLAGS) -c hparse_prob.c

hparse_deriv.o: hparse_deriv.c hparse.h hparse_hand.h
	$(CC) $(CFLAGS) -c hparse_deriv.c

hparse_agenda.o: hparse_agenda.c hparse.h hparse_hand.h
	$(CC) $(CFLAGS) -c hparse_agenda.c

syn.tab.o: syn.tab.c
	$(CC) $(CFLAGS) -c syn.tab.c 

syn.lex.o: syn.lex.c syn.tab.o
	$(CC) $(CFLAGS) -c syn.lex.c

syn.tab.c: syn.y syn_entry.h
	$(Y) $(YFLAGS) syn.y

syn.lex.c: syn.l syn.tab.o syn_entry.h common.h
	$(L) $(LFLAGS) syn.l > syn.lex.c

synclean:
	/bin/rm -f syn.lex.o 
	/bin/rm -f syn.tab.c syn.tab.h
	/bin/rm -f syn.lex.c
	/bin/rm -f $(SYNOBJ)

gram.o: gram.c gram.h gram_project.h
	$(CC) $(CFLAGS) -c gram.c

gram_slurp.o: gram_slurp.c gram.h
	$(CC) $(CFLAGS) -c gram_slurp.c

gram_trees.o: gram_trees.c gram.h
	$(CC) $(CFLAGS) -c gram_trees.c

gram_nodelist.o: gram_nodelist.c gram.h
	$(CC) $(CFLAGS) -c gram_nodelist.c

gram_treemap.o: gram_treemap.c gram.h
	$(CC) $(CFLAGS) -c gram_treemap.c

gram_treeinfo.o: gram_treeinfo.c gram.h
	$(CC) $(CFLAGS) -c gram_treeinfo.c

bin.o: bin.c bin.h
	$(CC) $(CFLAGS) -c bin.c

sent.o: sent.c sent.h
	$(CC) $(CFLAGS) -c sent.c

env.o: env.c env.h
	$(CC) $(CFLAGS) -c env.c

pref.o: pref.c pref.h pref_project.h pref_defn.h
	$(CC) $(CFLAGS) -c pref.c

corpus.o: corpus.c corpus.h
	$(CC) $(CFLAGS) -c corpus.c

err.o: err.c err.h
	$(CC) $(CFLAGS) -c err.c

mem.o: mem.c mem.h
	$(CC) $(CFLAGS) -c mem.c

spar.o: spar.c spar.h
	$(CC) $(CFLAGS) -c spar.c 

hash.o: hash.c hash.h
	$(CC) $(CFLAGS) -c hash.c

lex.o: lex.c lex.h
	$(CC) $(CFLAGS) -c lex.c 

lex_prune.o: lex_prune.c lex.h
	$(CC) $(CFLAGS) -c lex_prune.c 

lex_active.o: lex_active.c lex.h
	$(CC) $(CFLAGS) -c lex_active.c 

ia.o: ia.c ia.h
	$(CC) $(CFLAGS) -c ia.c 

heap.o: heap.c heap.h
	$(CC) $(CFLAGS) -c heap.c 

# with new Berkeley db
syn_db.o: syn_db.c syn_db.h
	$(CC) $(CFLAGS) $(DBINCLUDE) -c syn_db.c

prob_counts_db.o: prob_counts_db.c prob_counts_db.h
	$(CC) $(CFLAGS) $(DBINCLUDE) -c prob_counts_db.c

# --end

# without Berkeley db
syn_hash.o: syn_hash.c syn_hash.h
	$(CC) $(CFLAGS) $(DBINCLUDE) -c syn_hash.c

# --end

syn_entry.o: syn_entry.c syn_entry.h
	$(CC) $(CFLAGS) -c syn_entry.c 

clean: synclean
	/bin/rm -f *.o
	/bin/rm -f core
	/bin/rm -f mem.allocs
	/bin/rm -f mem.frees
	/bin/rm -f parser.bin
	/bin/rm -f nbest_parser.bin
	/bin/rm -f syn_get.bin
	/bin/rm -f Makefile

