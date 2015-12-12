
/* syn_db_old - interface to the legacy XTAG syntactic database */

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

static char rcsid[] = "$Id: syn_db_old.c,v 1.1 2000/06/06 16:15:24 anoop Exp $";

/* Not very portable.
   Requires libsyn.a, libhash.a and dynahash.o
   from the XTAG syntactic library.
   Also uses db.h and syn.h from that library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/* specific to the use of libhash and libsyn libraries */
#include <db.h>
#include <syn.h>

#include <assert.h>
#include "err.h"
#include "syn_db.h"

#include "env.h"
#define HOME "LEMHOME"

/* specified here cause the libsyn include files do not */
extern DB *C_open_syn_db(char *, DB *);
extern int C_close_syn_db(DB *);
extern int C_db_get_syn_decoded(const char *, DB *, char *);

void *
syn_db_open (char *file)
{
  DB *syndb = 0;

  env(HOME);

  err_debug("opening %s for use as word database\n", file);

  /* to avoid warnings created by syn.h */
  assert(END_DB_FIELD_STR);
  assert(SEP_STR);
  assert(SEP_STRE);

  syndb = C_open_syn_db(file, syndb);
  if (syndb == NULL) {
    err_abort("syn_db_open", "could not open db %s\n", file);
  }
  return((void *)syndb);
}

void
syn_db_close (void *syndb)
{
  env_delete();
  (void) C_close_syn_db((DB *)syndb);
}

int
syn_db (void *syndb, const char *windex, char *synbuffer)
{
  int status;

#ifdef DEBUG_SYN_DB
  err_warning("syn_db", "calling syn db for %s\n", windex);
#endif

  status = C_db_get_syn_decoded(windex, (DB *)syndb, synbuffer);

  if ((status != 0) && (!err_be_quiet())) {
      err_warning("syn_db", "no entry found for %s\n", windex);
  }

#ifdef DEBUG_SYN_DB
  err_warning("syn_db", "returned successfully\n");
#endif

  return(status ? 0 : 1); /* switch 0 to 1 */
}

