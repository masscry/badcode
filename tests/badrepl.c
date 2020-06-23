#include <badcode.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  FILE *input;
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  switch (argc)
  {
  case 1:
    input = stdin;
    break;
  case 2:
    input = fopen(argv[1], "r");
    if (input == NULL) 
    {
      perror("fopen");
      return EXIT_FAILURE;
    }
    break;
  default:
    fprintf(stderr, "Usage: %s [<file>]\n", argv[0]);
    return EXIT_FAILURE;
  }

  BC_CORE core = NULL;
  bcStatus_t status = bcCoreNew(&core);
  if (status != BC_OK)
  {
    fprintf(stderr, "bcCoreNew failed: %d\n", status);
    return EXIT_FAILURE;
  }
  
  if (input == stdin)
  {
    fprintf(stdout, "%s", ">>> ");
    fflush(stdout);
  }

  while ((nread = getline(&line, &len, input)) != -1) 
  {
    status = bcCoreExecute(core, line, NULL);
    if (status == BC_PARSE_NOT_FINISHED)
    {
      fprintf(stdout, "%s", "... ");
      fflush(stdout);
      continue;
    }

    if (status != BC_OK)
    {
      fprintf(stderr, "Error: %d\n", status);
      fprintf(stdout, "%s", ">>> ");
      fflush(stdout);
      continue;
    }

    BC_VALUE top;
    while(bcCoreTop(core, &top) == BC_OK)
    {
      bcValuePrint(stdout, top);
      fprintf(stdout, "\n");
      bcCorePop(core);
    }

    fprintf(stdout, "%s", ">>> ");
    fflush(stdout);
  }

  bcCoreDelete(core);

  free(line);
  if (input != stdin)
  {
    fclose(input);
  }
  return EXIT_SUCCESS;
}
