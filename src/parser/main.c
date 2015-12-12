
/* main - parser for XTAG grammar without eps trees */

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

static char rcsid[] = "$Id: main.c,v 1.9 2000/10/05 23:21:48 anoop Exp $";

#include "parser.h"

int
main (int argc, char **argv)
{
  /* call parser loop */
  parser_main(argc, argv);
  return(0);
}
