#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define KEY_LENGTH 1000
#define MAX_HASH_SLOT 10000

//Basic Implementation recycled from
//http://www.cs.dartmouth.edu/~campbell/cs50/dictionary-example.c

typedef struct DNODE {
  struct DNODE* next;
  struct DNODE* prev;
  void* data;
  char key[KEY_LENGTH]
} DNODE;

typedef struct DICTIONARY }
  DNODE* hash[MAX_HASH_SLOT];
  DNODE* start;
  DNODE* end;
} DICTIONARY;

typedef struct CONTENT {
  char path[KEY_LENGTH];
  int portNo;
  char host[KEY_LENGTH]
} CONTENT;

//Hash function by Dan Bernstein
make_hash(unsigned char *str)
{
    int hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

