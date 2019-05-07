#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct arg_pair arg_pair;

struct arg_pair {
  char *key;
  char *value;
  arg_pair *next;
};

arg_pair *free_pair_list(arg_pair *pair)
{
  arg_pair *tmp;
  while ( pair != NULL )
  {
    tmp = pair;
    pair = pair->next;
    if(tmp->key) {
      free(tmp->key);
    }
    if(tmp->value) {
      free(tmp->value);
    }
    free(tmp);
  }
}

char *get_key_pair(char *query_string, int start_offset)
{
  int i = 0;
  char *str = query_string + start_offset;
  while(i < strlen(str) && str[i] != '&' ) {
    i++;
  }
  char *a_key_pair = strndup(str, i );
  return a_key_pair;
}


/* pass mkargpair("name=val") to assign name and val*/
arg_pair *mkargpair(char *str)
{
  int keylen=1; /* first char cannot be '=' since that'd be a key name */
  int keylenfound=0;
  while (keylen < strlen(str) && !keylenfound) {
    if(str[keylen]=='=') {
      if(keylen > 1) {
        keylenfound=keylen;
      }
      else {
        /* INVALID FORMAT, but we still need this loop to exit. */
        keylenfound=-1;
      }
    }
    keylen++;
  }
  arg_pair *newpair = malloc(sizeof(struct arg_pair));
  newpair->next = NULL;
  newpair->key = strndup(str,keylenfound);
  newpair->value = strndup(str+keylen,strlen(str)-keylen);
#ifdef DEBUG
  printf("key: %s, size: %d\r\n",newpair->key, strlen(newpair->key));
  printf("value: %s, size: %d\r\n",newpair->value, strlen(newpair->key));
#endif
  
  return newpair;
}

void tokenize_query(char *query_string, arg_pair *dest){
#ifdef DEBUG
    printf("QUERY_STRING=%s\r\n",*query_string);
#endif

    /* pass one name=value pair per call to mkargpair() */

    /* get first key pair */
    int offset=0;
    char *a_pair;
    arg_pair *root_keypair=dest; /* pointer to start of linked list */
    arg_pair *curr_pair=NULL;
    arg_pair *last_pair=NULL;
    while(offset < strlen(query_string))
    {
      a_pair = get_key_pair(query_string, offset); /* remember - this does a malloc */
#ifdef DEBUG
      printf("a_pair: %s\r\n", a_pair);
#endif
      offset=offset+strlen(a_pair)+1;

      if(curr_pair==NULL) {
        /* first run. Since we're passing pointers all over the place, we need
           to do some pointer sorcery on the root (basically, all the members of
           our first run from mkargpair() need to be in the dest struct, so that
           they can be used outside of this function.)
        */
        curr_pair = mkargpair(a_pair);
        dest->key=curr_pair->key;
        dest->value=curr_pair->value;
        dest->next=curr_pair->next;
        free(curr_pair);
        curr_pair=dest;
        root_keypair = curr_pair;
      }
      else
      {
        curr_pair->next = mkargpair(a_pair);
        curr_pair = curr_pair->next;
      }

      free(a_pair); /* free the malloc from get_key_pair */
    }
    dest->next=root_keypair->next;
}

int main(int argc, char **argv) {
  printf("Content-Type: text/plain;charset=utf-8\r\n\r\n");
  const char *query_string = getenv("QUERY_STRING");
  if( !query_string ) /* if no arguments/undefined variable */
  {
    printf("No parameters were passed…\r\n");
    return (0);
  }
  else
  {
    /* initialize our root node */
    arg_pair *args = malloc(sizeof(struct arg_pair));
    /* convert our parameters into key/value linked list*/
    tokenize_query(query_string, args);
    int i=1;
    arg_pair *arg_ptr = args; /* Don't lose our root node pointer! */
    while(arg_ptr)
    {
      printf("%d:\t%s = %s\r\n", i, arg_ptr->key, arg_ptr->value);
      arg_ptr = arg_ptr->next;
      i++;
    }
    free_pair_list(args);
  }
  return 0;
}

