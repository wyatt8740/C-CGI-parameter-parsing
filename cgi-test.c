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

int main(int argc, char **argv) {
  printf("Content-Type: text/plain;charset=utf-8\r\n\r\n");
  const char *query_string=getenv("QUERY_STRING");
  if( !query_string ) /* if no arguments/undefined variable */
  {
    printf("No parameters were passed…\r\n");
    return (0);
  }
  else
  {
#ifdef DEBUG
    printf("QUERY_STRING=%s\r\n",query_string);
#endif

    /* pass one name=value pair per call to mkargpair() */

    /* get first key pair */
    int offset=0;
    char *a_pair;
    arg_pair *root_keypair=NULL; /* pointer to start of linked list */
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
        curr_pair = mkargpair(a_pair);
        root_keypair = curr_pair;
      }
      else
      {
        curr_pair->next = mkargpair(a_pair);
        curr_pair = curr_pair->next;
      }

      free(a_pair); /* free the malloc from get_key_pair */
#ifdef DEBUG
      printf("MAIN FUNCTION:\r\n");
#endif
      printf("key:\t%s\r\n",curr_pair->key);
      printf("value:\t%s\r\n",curr_pair->value);
      printf("\r\n");
    }


    curr_pair=root_keypair;
    free_pair_list(curr_pair);
    return (0);
  }
}
