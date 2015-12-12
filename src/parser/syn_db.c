
/* syn_db - interface to the XTAG syntactic database */

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

static char rcsid[] = "$Id: syn_db.c,v 1.2 2000/11/02 02:24:53 anoop Exp $";

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <db.h>

#include "mem.h"
#include "err.h"

extern int errno;

void *
syn_db_open (char *db_file)
{
  DB *dbp;
  db_create(&dbp, 0, 0);
  if ((errno = dbp->open(dbp, db_file, NULL, DB_HASH, DB_RDONLY, 0664)) != 0) {
    err_abort("syn_db_open", "could not open database %s: %s\n", 
	      db_file, strerror(errno));
  }
  return((void *)dbp);
}

int 
syn_db (void *vp, char *key_data, char *buf)
{
  DB *dbp;
  DBT key, data;
  DBC *cur;
  int status = 0;

  dbp = (DB *)vp;
  buf[0] = '\0';

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));

  /* key.data = mem_strdup(key_data); */
  key.data = key_data;
  key.size = strlen(key_data) + 1;
  data.flags = DB_DBT_MALLOC;

#ifdef DEBUG_SYN_DB
  err_warning("syn_db", "key data = %s\n", (char *)key.data);
  err_warning("syn_db", "key size = %d\n", key.size);
  err_warning("syn_db", "searching for: <%s>\n", 
	      (char *)key.data);
#endif

  switch (errno = dbp->cursor(dbp, NULL, &cur, 0)) {

  case 0:

#ifdef DEBUG_SYN_DB
    err_warning("syn_db", "created cursor\n");
#endif

    break;

  default: 
    err_abort("syn_db", "cursor alloc error: %s\n", 
	      strerror(errno));
    exit (1);
  }

  switch ((errno = cur->c_get(cur, &key, &data, DB_SET))) {

  case 0:
    strcpy(buf, data.data);
    free(data.data);
    status = 1;
    break;

  case DB_NOTFOUND:

    if (!err_be_quiet()) {
      err_warning("syn_db", "no entry found for %s\n", (char *)key.data);
    }
    status = 0;
    break;

  default:
    err_abort("syn_db", "get: %s\n", strerror(errno));
  }

  while ((errno = cur->c_get(cur, &key, &data, DB_NEXT_DUP)) == 0) {
    switch(errno) {

    case 0:
      strcat(buf, data.data);
      free(data.data);
      status = 1;
      break;

    case DB_NOTFOUND:

      if (!err_be_quiet()) {
        err_warning("syn_db", "no entry found for %s\n", (char *)key.data);
      }
      status = 0;
      break;

    default:
      err_abort("syn_db", "get: %s\n", strerror(errno));
    }
  }

  /* mem_free(key.data); */

  switch (errno = cur->c_close(cur)) {
  case 0:

#ifdef DEBUG_SYN_DB
    err_warning("syn_db", "cursor released\n");
#endif

    break;
  default:
    err_abort("syn_db", "get: %s\n", strerror(errno));
  }

  return(status);
}

void
syn_db_close (void *vbp)
{
  DB *dbp;

  dbp = (DB *)vbp;
  switch ((errno = dbp->close(dbp, 0))) {

  case 0:

#ifdef DEBUG_SYN_DB
    err_warning("syn_db", "database closed\n");
#endif

    break;

  default:
    err_abort("syn_db", "error closing database: %s\n", 
	      strerror(errno));
  }
}

