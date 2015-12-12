
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

static char rcsid[] = "$Id: err.c,v 1.2 2000/10/05 22:18:10 anoop Exp $";

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "err.h"

void 
err_abort (const char *function_name, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  (void) fprintf(stderr, "Error in %s: ", function_name);
  (void) vfprintf(stderr, format, ap);
  va_end(ap);
  err_die();
}

void 
err_message (const char *function_name, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  (void) fprintf(stderr, "Error in %s: ", function_name);
  (void) vfprintf(stderr, format, ap);
  va_end(ap);
}

void 
err_warning (const char *function_name, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  (void) fprintf(stderr, "Warning from %s: ", function_name);
  (void) vfprintf(stderr, format, ap);
  va_end(ap);
}

void 
err_debug (const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  (void) vfprintf(stderr, format, ap);
  va_end(ap);
}

void 
err_print (const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  (void) vfprintf(stdout, format, ap);
  va_end(ap);
}

void 
err_hint (const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  (void) fprintf(stderr, "Hint: ");
  (void) vfprintf(stderr, format, ap);
  va_end(ap);
}

void
err_die ()
{
  (void) abort;
  exit(1);
}
