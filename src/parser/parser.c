
/* parser - generic control loop for the parser */

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

static char rcsid[] = "$Id: parser.c,v 1.5 2001/07/06 22:29:08 anoop Exp $";

#include <stdio.h>
#include <time.h>
#include "mem.h"
#include "err.h"
#include "hparse.h"
#include "pref.h"

void
parser_main (int argc, char **argv)
{
  hand_t *hand;
  hparse_t *hp;
  unsigned unparsed = 0;
  unsigned total = 0;
  clock_t c0, c1;

  if (argc != 3) {
    err_abort("usage", "%s <grammar> <preferences>\n", argv[0]);
  }

#ifdef DEBUG_MEM
  mem_debug_init();
#endif

  read_prefs(argv[2]);

  /* set up buffering prefs */
  if (_buffer_stdout == 0) {
    setvbuf(stdout, NULL, _IONBF, 0);
  }

  if (_buffer_stderr == 0) {
    setvbuf(stderr, NULL, _IONBF, 0);
  }

  hand = hand_new(argv[1], stdin);
  err_debug("ready...\n");

  c0 = clock();
  while ((hp = hparse_main(hand)))
    {
      err_debug("done parsing...\n");
      c1 = clock();
      err_debug("time(CPU): %lf\n", (double) (c1 - c0)/CLOCKS_PER_SEC);
      unparsed += hparse_derivation_handler(hp, hand, forest);
      total++;
      hparse_cleanup(hand, hp);
      c0 = clock();
    }
  hand_delete(hand);

#ifdef DEBUG_MEM
  mem_debug_close();
  mem_report();
#endif

  err_debug("parsed %d from a total of %d\n", total - unparsed, total);
}

