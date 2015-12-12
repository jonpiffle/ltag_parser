
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <db.h>

#define SYN_BUFSZ   32768

char syn_buf[SYN_BUFSZ];
extern int errno;

DB *
syn_db_open (char *db_file)
{
  DB *dbp;
  if ((errno = db_open(db_file, DB_HASH, DB_RDONLY, 
		       0664, NULL, NULL, &dbp)) != 0) {
    fprintf(stderr, "db: %s: %s\n", db_file, strerror(errno));
    exit (1);
  }
  return(dbp);
}

int 
syn_db_access (DB *dbp, char *key_data, char *buf)
{
  DBT key, data;
  DBC *cur;
  int status = 0;

  buf[0] = '\0';

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));

  /* key.data = strdup(key_data); */
  key.data = key_data;
  key.size = strlen(key_data) + 1;
  data.flags = DB_DBT_MALLOC;

  fprintf(stderr, "db: key data = %s\n", (char *)key.data);
  fprintf(stderr, "db: key size = %d\n", key.size);

  fprintf(stderr, "db: searching for: <%s>\n", 
	  (char *)key.data);

  switch (errno = dbp->cursor(dbp, NULL, &cur, 0)) {
  case 0:
    fprintf(stderr, "db: created cursor\n");
    break;
  default: 
    fprintf(stderr, "db: cursor: %s\n", strerror(errno));
    exit (1);
  }

  switch ((errno = cur->c_get(cur, &key, &data, DB_SET))) {
  case 0:
    strcpy(buf, data.data);
    free(data.data);
    status = 1;
    break;
  case DB_NOTFOUND:
    fprintf(stderr, "db: %s: key not found.\n", (char *)key.data);
    status = 0;
    break;
  default:
    fprintf(stderr, "db: get: %s\n", strerror(errno));
    exit (1);
  }

  while ((errno = cur->c_get(cur, &key, &data, DB_NEXT_DUP)) == 0) {
    switch(errno) {
    case 0:
      strcat(buf, data.data);
      free(data.data);
      break;
    case DB_NOTFOUND:
      printf("db: %s: key not found.\n", (char *)key.data);
      break;
    default:
      fprintf(stderr, "db: get: %s\n", strerror(errno));
      exit (1);
    }
  }

  switch (errno = cur->c_close(cur)) {
  case 0:

#ifdef DEBUG_SYN_DB
    err_warning("syn_db", "cursor released\n");
#endif

    break;
  default:
      fprintf(stderr, "db: get: %s\n", strerror(errno));
      exit (1);
  }

  /* free(key.data); */
  return(status);
}

void
syn_db_close (DB *dbp)
{
  switch ((errno = dbp->close(dbp, 0))) {
  case 0:
    fprintf(stderr, "db: database closed.\n");
    break;
  default:
    fprintf(stderr, "db: get: %s\n", strerror(errno));
    exit (1);
  }
}

int
main(int argc, char **argv)
{
  DB *dbp;
  int status = 0;

  if (argc != 3) {
    fprintf(stderr, "usage: %s <db> <word>\n", argv[0]);
    exit(1);
  }

  dbp = syn_db_open(argv[1]);
  status = syn_db_access(dbp, argv[2], syn_buf);
  printf("status=%d\nsyn_buf=%s\n", status, syn_buf);
  syn_db_close(dbp);

  return(status);
}

