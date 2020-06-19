#include <bcPrivate.h>

#include <stdlib.h>

BCAPI bcStatus_t bcValueCleanup(BC_VALUE value)
{
  if (value == NULL)
  {
    return BC_INVALID_ARG;
  }

  switch (value->type)
  {
  case BC_INTEGER:
  case BC_NUMBER:
    free(value);
    return BC_OK;
  default:
    return BC_NOT_IMPLEMENTED;
  }
}

BCAPI BC_VALUE bcValueCopy(const BC_VALUE val)
{
  if (val == NULL)
  {
    return NULL;
  }

  switch (val->type)
  {
  case BC_INTEGER:
    return bcValueInteger(((bcInteger_t*) val)->data);
  case BC_NUMBER:
    return bcValueNumber(((bcNumber_t*) val)->data);
  default:
    return NULL;
  }
}

BCAPI BC_VALUE bcValueInteger(int64_t val)
{
  bcInteger_t* result = (bcInteger_t*) malloc(sizeof(bcInteger_t));
  if (result == NULL)
  {
    return NULL;
  }

  result->head.type = BC_INTEGER;
  result->data = val;

  return &result->head;
}

BCAPI BC_VALUE bcValueNumber(double val)
{
  bcNumber_t* result = (bcNumber_t*) malloc(sizeof(bcNumber_t));
  if (result == NULL)
  {
    return NULL;
  }

  result->head.type = BC_NUMBER;
  result->data = val;
  return &result->head;
}

BCAPI bcStatus_t bcValueAsInteger(const BC_VALUE val, int64_t* oval)
{
  if ((val == NULL) || (oval == NULL))
  {
    return BC_INVALID_ARG;
  }

  switch (val->type)
  {
  case BC_INTEGER:
    {
      const bcInteger_t* ival = (const bcInteger_t*)val;
      *oval = ival->data;
      return BC_OK;
    }
    break;
  case BC_NUMBER:
    {
      const bcNumber_t* nval = (const bcNumber_t*)val;
      *oval = (int64_t) nval->data;
      return BC_OK;
    }
    break;
  default:
    return BC_NOT_IMPLEMENTED;
  }
}

BCAPI bcStatus_t bcValueAsNumber(const BC_VALUE val, double* oval)
{
  if ((val == NULL) || (oval == NULL))
  {
    return BC_INVALID_ARG;
  }

  switch (val->type)
  {
  case BC_INTEGER:
    {
      const bcInteger_t* ival = (const bcInteger_t*)val;
      *oval = (double) ival->data;
      return BC_OK;
    }
    break;
  case BC_NUMBER:
    {
      const bcNumber_t* nval = (const bcNumber_t*)val;
      *oval = nval->data;
      return BC_OK;
    }
    break;
  default:
    return BC_NOT_IMPLEMENTED;
  }  
}

BCAPI int bcValuePrint(FILE* stream, const BC_VALUE val)
{
  switch (val->type)
  {
  case BC_INTEGER:
    return fprintf(stream, "%ld", ((const bcInteger_t*)val)->data);
  case BC_NUMBER:
    return fprintf(stream, "%g", ((const bcNumber_t*)val)->data);
  default:
    return fprintf(stream, "%s", "NOT-IMPLEMENTED");
  }

}
