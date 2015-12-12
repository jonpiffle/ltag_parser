
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "nl-java.h"
#include "nl-list.h"
#include "nl-stringassoc.h"
#include "nl-featurenode.h"
#include "nl-feattable.h"
//#include "nl-morph.h"
//#include "nl-grammar.h"
//#include "nl-lexicon.h"
//#include "nl-instanchor.h"
//#include "nl-chart.h"
//#include "nl-mcchart.h"
//#include "nl-transfer.h"
//#include "nl-forest.h"
//#include "nl-mcforest.h"
//#include "nl-interface.h"

#define ALPHANUM(c)            ((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9'))
#define SPACE(c)               (c==' ')
#define WHITESPACE(c)          (c==' ' || c=='\t' || c=='\n')
#define CONSUME_OPT(c,f,b)     c = (b) ? getc(f) : c
#define CONSUME_ONE(c,f,b)     c = (b) ? getc(f) : c+(0*fprintf(stderr,"ERROR: Missing delimiter.\n"))
#define CONSUME_ALL(c,f,b)     for ( ; b; c=getc(f) )
#define CONSUME_STR(c,f,b,s,i) for ( i=0; (b) || false != (s[i++]='\0'); s[i++]=c, c=getc(f) )


