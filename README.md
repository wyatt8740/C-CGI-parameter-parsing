# CGI: Parameter Parsing in C

This project intends to tokenize URI parameters for use in a CGI program
written in C.

Why? I don't really know. Just kind of felt like trying out CGI in 2019, and
also felt like playing around in C a bit more. I don't know if anything will
ever come of this.

This program will create a C structure typedef'd as the type `arg_pair`, which
follows this scheme:

```c
struct arg_pair {
  char *key;
  char *value;
  arg_pair *next;
};
```

As you can probably see, this means that all arguments (as seen in the URI in
in key=value form) are stored in a linked list.

For a URI like `http://domain/path/to/cgi.cgi?name=wyatt%20ward`, The result would be
a linked list one level deep containing `args->key="name"` and
`args->value="wyatt ward"`.

To find a key `name`'s value, one might try something like this
(in the main method, anyway):

```c
int found=0;
arg_pair *arg_ptr = args; /* Don't lose our root node pointer! */
while(arg_ptr && !found) {
  if( strcmp( "name", arg_ptr->key ) ) {
    printf("I see your name is %s\r\n", arg_ptr->value);
    found=1;
  }
  else {
    arg_ptr = arg_ptr->next;
  }
}
```

Valgrind has detected no memory leaks in this program.

### References

Stole this URI decoding function from: Stack Overflow:
https://stackoverflow.com/questions/2673207/c-c-url-decode-library/14530993

Other than that, all code is my own.