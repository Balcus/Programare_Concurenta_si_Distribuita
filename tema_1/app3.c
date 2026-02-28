#include <argtable3.h>
#include <stdio.h>
#include <stdlib.h>
// Just for test to see if conan works!

int main(int argc, char *argv[]) {
  // define the options and the arguments
  // short option followed by a long option, followed by the value type
  // and the explanation

  // similar to argparse from python
  struct arg_int *a = arg_int1("a", "nr1", "<int>", "First term");
  struct arg_int *b = arg_int1("b", "nr2", "<int>", "Second term");
  struct arg_end *end = arg_end(20);

  void *argtable[] = {a, b, end};

  // check table integrity
  if (arg_nullcheck(argtable) != 0) {
    fprintf(stderr, "Error allocating memory for argtable!\n");
    return 1;
  }

  // parse the CL and get the options and their values
  int nerrors = arg_parse(argc, argv, argtable);

  // if errors then report them and exit
  if (nerrors > 0) {
    arg_print_errors(stderr, end, argv[0]);
    printf("Usage: %s", argv[0]);
    arg_print_syntax(stderr, argtable, "\n");
    return 1;
  }

  // no errors mean we can continue working with the extrated values
  int sum = a->ival[0] + b->ival[0];
  printf("%d + %d = %d\n", a->ival[0], b->ival[0], sum);

  // free the arg table at the end
  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
  return 0;
}
