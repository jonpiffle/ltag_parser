
/* prob_counts_db - interface to the model counts database */

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

#ifndef _PROB_COUNTS_DB_H
#define _PROB_COUNTS_DB_H

extern void *prob_counts_db_open (const char *);
extern void prob_counts_db_close (void *);
extern int prob_counts_db (void *, char *, char *);

#endif /* _PROB_COUNTS_DB_H */

