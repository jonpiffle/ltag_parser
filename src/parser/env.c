
/* env - insert several environment variables relative to home */

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

static char rcsid[] = "$Id: env.c,v 1.1 2000/06/06 16:03:33 anoop Exp $";

#include <stdlib.h>
#include <string.h>
#include "myassert.h"
#include "mem.h"
#include "err.h"
#include "env.h"

#define NUM_ENV 4

static inline size_t
envlen (const char *var, const char *dir, const char *file)
{
  return(strlen(var)+strlen(dir)+strlen(file));
}

static inline void
envcpy (char *ptr, const char *var, const char *dir, const char *file)
{
  strcpy(ptr,var); strcat(ptr,dir); strcat(ptr,file);
}

/*
 #define envlen(var,dir,file)     strlen(var)+strlen(dir)+strlen(file)
 #define envcpy(ptr,var,dir,file) strcpy(ptr,var); strcat(ptr,dir); strcat(ptr,file);
*/

static struct env_tbl_s {
  const char *var;
  const char *value;
  char *env_string;
} env_tbl[NUM_ENV] = {
    {     "FS_CODES=", "/data/code_fs.txt" ,      0},
    {   "TREE_CODES=", "/data/code_trees.txt" ,   0},
    { "FAMILY_CODES=", "/data/code_families.txt", 0},
    {    "POS_CODES=", "/data/code_pos.txt" ,     0}
};

char *
env_new (const char *env_var, const char *home_dir, const char *file)
{
  char *env_string;

  env_string = (char *) mem_alloc((envlen(env_var,home_dir,file)+1) * sizeof(char));
  myassert(env_string);
  envcpy(env_string,env_var,home_dir,file);
  return(env_string);
}

void
env_delete ()
{
  int i;
  for (i = 0; i < NUM_ENV; i++)
    {
      mem_free(env_tbl[i].env_string);
    }
}

void
env (const char *home_var)
{
  char *home_dir;
  int i;

  if ((home_dir = getenv(home_var)) == NULL) {
    err_abort("env", "could not get value of %s\n", home_var);
  }

  for (i = 0; i < NUM_ENV; i++)
    {
      env_tbl[i].env_string = env_new(env_tbl[i].var, home_dir, env_tbl[i].value);

#ifdef DEBUG_ENV
      err_debug("inserting into env: %s\n", env_tbl[i].env_string);
#endif

      if (putenv(env_tbl[i].env_string)) {
	err_abort("env", "could not insert into environment: %s\n", env_tbl[i].env_string);
      }
    }
}
