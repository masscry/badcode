#include <badcode.h>

#include <assert.h>

int main(int argc, char* argv[])
{
  uint32_t version = bcVersion();
  assert(version == BC_VER_FULL);

  BC_CORE bcCore = NULL;
  bcStatus_t status;

  status = bcCoreNew(&bcCore);

  assert(status == BC_OK);
  assert(bcCore != NULL);

  status = bcCoreExecute(NULL,
    "a = 100 + 10.0",
    NULL
  );

  assert(status == BC_INVALID_ARG);

  status = bcCoreExecute(bcCore,
    NULL,
    NULL
  );

  assert(status == BC_INVALID_ARG);

  status = bcCoreExecute(bcCore,
    "100 + 10.5",
    NULL
  );
  assert(status == BC_OK);

  char* lastCmd = NULL;
  status = bcCoreExecute(bcCore,
    "100 + 10.5",
    &lastCmd
  );
  assert(status == BC_OK);
  assert(*lastCmd == '\0');

  BC_VALUE result = NULL;
  status = bcCoreTop(bcCore, &result);
  assert(status == BC_OK);
  assert(result != NULL);

  double topValue;

  status = bcValueAsNumber(result, &topValue);

  assert(status == BC_OK);

  assert(topValue == 110.5);

  bcCoreDelete(bcCore);
  return 0;
}