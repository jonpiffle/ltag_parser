
/* err - error management */

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

#ifndef _ERR_H
#define _ERR_H

#include "pref.h"

extern void err_abort (const char *, const char *, ...);
extern void err_message (const char *, const char *, ...);
extern void err_warning (const char *, const char *, ...);
extern void err_debug (const char *, ...);
extern void err_print (const char *format, ...);
extern void err_hint (const char *, ...);
extern void err_die ();

static inline int
err_be_quiet (void)
{
  return(_err_quiet);
}

#endif /* _ERR_H */
