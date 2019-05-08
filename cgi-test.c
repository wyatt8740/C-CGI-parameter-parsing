#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct arg_pair arg_pair;

struct arg_pair {
  char *key;
  char *value;
  arg_pair *next;
};

char *get_key_pair(char *query_string, int start_offset);
arg_pair *free_pair_list(arg_pair *pair);
arg_pair *mkargpair(char *str);
void tokenize_query(char *query_string, arg_pair *dest);
/* following function stolen from:
   https://stackoverflow.com/questions/2673207/c-c-url-decode-library/14530993
*/
void urldecode(char *dst, const char *src);

/* TODO: consider using this instead:
   https://github.com/wcharczuk/urlencode/blob/master/urldecode.c
*/

int main(int argc, char **argv) {
  printf("Content-Type: text/plain;charset=utf-8\r\n\r\n");
  const char *query_string = getenv("QUERY_STRING");
  if( !query_string ) { /* if no arguments/undefined variable */
    printf("No parameters were passed…\r\n");
    return (0);
  }
  else {
    /* initialize our root node */
    arg_pair *args = malloc(sizeof(struct arg_pair));
    /* convert our parameters into key/value linked list*/
    tokenize_query(query_string, args);

    /* Iterate through and print out all argument keys and their values. */
    int i=1;
    arg_pair *arg_ptr = args; /* Don't lose our root node pointer! */
    while(arg_ptr) {
      printf("%d:\t%s = %s\r\n", i, arg_ptr->key, arg_ptr->value);
      arg_ptr = arg_ptr->next;
      i++;
    }
    free_pair_list(args);
  }
  return (0);
}


arg_pair *free_pair_list(arg_pair *pair)
{
  arg_pair *tmp;
  while ( pair != NULL ) {
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
arg_pair *mkargpair(char *str) {
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
  /* un-escape escaped characters. */
  urldecode(newpair->key, newpair->key);
    /* PRETTY SURE THIS WILL LEAVE A FEW TRAILING NULL BYTES POTENTIALLY.
       WASTED SPACE */

  
  newpair->value = strndup(str+keylen,strlen(str)-keylen);
  urldecode(newpair->value, newpair->value);

  return newpair;
}

void tokenize_query(char *query_string, arg_pair *dest){
    /* pass one name=value pair per call to mkargpair() */

    /* get first key pair */
    int offset=0;
    char *a_pair; /* a_pair is a string containing a single 'key=value' entry. */
    arg_pair *root_keypair=dest; /* pointer to start of linked list */
    arg_pair *curr_pair=NULL;
    while(offset < strlen(query_string)) {
      a_pair = get_key_pair(query_string, offset); /* remember - this does a malloc */
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
      else {
        curr_pair->next = mkargpair(a_pair);
        curr_pair = curr_pair->next;
      }
      free(a_pair); /* free the malloc from get_key_pair */
    }
    dest->next=root_keypair->next;
}

void urldecode(char *dst, const char *src)
{
  char a, b;
  while (*src) {
    if ((*src == '%') &&
        ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a'-'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a'-'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      *dst++ = 16*a+b;
      src+=3;
/* not necessary, I think. %20 is a space already: */
/*    } else if (*src == '+') {
      *dst++ = ' ';
      src++;*/
    } else {
      *dst++ = *src++;
    }
  }
  *dst++ = '\0';
}
