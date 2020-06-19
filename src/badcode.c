#include <bcPrivate.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

uint32_t bcVersion()
{
  return BC_VER_FULL;
}

bcStatus_t bcCoreNew(BC_CORE* pCore)
{
  if (pCore == NULL)
  {
    return BC_INVALID_ARG;
  }

  BC_CORE result = (BC_CORE) malloc(sizeof(struct bcCore_t));
  if (result == NULL)
  {
    return BC_NO_MEMORY;
  }

  bcStatus_t status = bcValueStackInit(&result->stack, BC_CORE_VALUE_STACK_SIZE);
  if (status != BC_OK)
  {
    free(result);
    return status;
  }

  *pCore = result;
  return BC_OK;
}

void bcCoreDelete(BC_CORE core)
{
  if (core != NULL)
  {
    bcValueStackCleanup(&core->stack);
    free(core);
  }
}

bcDataType_t bcPromote(const BC_VALUE a, const BC_VALUE b)
{
  switch (a->type)
  {
  case BC_INTEGER:
    switch (b->type)
    {
    case BC_INTEGER:
      return BC_INTEGER;
    case BC_NUMBER:
      return BC_NUMBER;
    default:
      return BC_NULL;
    }
  case BC_NUMBER:
    switch (b->type)
    {
    case BC_INTEGER:
    case BC_NUMBER:
      return BC_NUMBER;
    default:
      return BC_NULL;
    }
  default:
    if (b->type != a->type)
    { // invalid conversion
      return BC_NULL;
    }
    return a->type;
  }
}

static bcStatus_t bcValueBinaryOperatorAlgebra(const BC_VALUE a, const BC_VALUE b, uint8_t binop, BC_VALUE* result)
{
  assert((result != NULL) && (a != NULL) && (b != NULL));

  bcDataType_t promotedType = bcPromote(a, b);
  if (promotedType == BC_NULL)
  {
    return BC_NOT_IMPLEMENTED;
  }

  switch (promotedType)
  {
    case BC_NULL:
    default:
      return BC_NOT_IMPLEMENTED;
    case BC_INTEGER:
      {
        int64_t aVal;
        int64_t bVal;
        bcValueAsInteger(a, &aVal);
        bcValueAsInteger(b, &bVal);
        switch (binop)
        {
        case BC_ADD:
          aVal += bVal;
          break;
        case BC_SUB:
          aVal -= bVal;
          break;
        case BC_MUL:
          aVal *= bVal;
          break;
        case BC_DIV:
          aVal /= bVal;
          break;
        case BC_MOD:
          aVal %= bVal;
          break;
        default:
          return BC_NOT_IMPLEMENTED;
        }
        *result = bcValueInteger(aVal);
        return BC_OK;
      }
    case BC_NUMBER:
      {
        double aVal;
        double bVal;
        bcValueAsNumber(a, &aVal);
        bcValueAsNumber(b, &bVal);
        switch (binop)
        {
        case BC_ADD:
          aVal += bVal;
          break;
        case BC_SUB:
          aVal -= bVal;
          break;
        case BC_MUL:
          aVal *= bVal;
          break;
        case BC_DIV:
          aVal /= bVal;
          break;
        case BC_MOD:
          aVal = fmod(aVal, bVal);
          break;
        default:
          return BC_NOT_IMPLEMENTED;
        }
        *result = bcValueNumber(aVal);
        return BC_OK;
      }
  }
}

static bcStatus_t bcValueBinaryOperatorCompare(const BC_VALUE a, const BC_VALUE b, uint8_t binop, BC_VALUE* result)
{
  assert((result != NULL) && (a != NULL) && (b != NULL));

  bcDataType_t promotedType = bcPromote(a, b);
  if (promotedType == BC_NULL)
  {
    return BC_NOT_IMPLEMENTED;
  }

  switch (promotedType)
  {
    case BC_NULL:
    default:
      return BC_NOT_IMPLEMENTED;
    case BC_INTEGER:
      {
        int64_t aVal;
        int64_t bVal;
        int64_t cmpResult;
        bcValueAsInteger(a, &aVal);
        bcValueAsInteger(b, &bVal);
        switch (binop)
        {
        case BC_EQ:
          cmpResult = (aVal == bVal);
          break;
        case BC_NEQ:
          cmpResult = (aVal != bVal);
          break;
        case BC_GR:
          cmpResult = (aVal > bVal);
          break;
        case BC_LS:
          cmpResult = (aVal < bVal);
          break;
        case BC_GRE:
          cmpResult = (aVal >= bVal);
          break;
        case BC_LSE:
          cmpResult = (aVal <= bVal);
          break;
        default:
          return BC_NOT_IMPLEMENTED;
        }
        *result = bcValueInteger(cmpResult);
        return BC_OK;
      }
    case BC_NUMBER:
      {
        double aVal;
        double bVal;
        int64_t cmpResult;
        bcValueAsNumber(a, &aVal);
        bcValueAsNumber(b, &bVal);
        switch (binop)
        {
        case BC_EQ:
          cmpResult = (aVal == bVal);
          break;
        case BC_NEQ:
          cmpResult = (aVal != bVal);
          break;
        case BC_GR:
          cmpResult = (aVal > bVal);
          break;
        case BC_LS:
          cmpResult = (aVal < bVal);
          break;
        case BC_GRE:
          cmpResult = (aVal >= bVal);
          break;
        case BC_LSE:
          cmpResult = (aVal <= bVal);
          break;
        default:
          return BC_NOT_IMPLEMENTED;
        }
        *result = bcValueInteger(cmpResult);
        return BC_OK;
      }
  }
}

static bcStatus_t bcValueBinaryOperatorLogicBitwise(const BC_VALUE a, const BC_VALUE b, uint8_t binop, BC_VALUE* result)
{
  assert((result != NULL) && (a != NULL) && (b != NULL));

  if ((a->type != BC_INTEGER) || (b->type != BC_INTEGER))
  {
    return BC_NOT_IMPLEMENTED;
  }
  int64_t aVal;
  int64_t bVal;

  bcValueAsInteger(a, &aVal);
  bcValueAsInteger(b, &bVal);

  switch (binop)
  {
  case BC_LND:
    aVal = (aVal != 0) && (bVal != 0);
    break;
  case BC_LOR:
    aVal = (aVal != 0) || (bVal != 0);
    break;
  case BC_BND:
    aVal &= bVal;
    break;
  case BC_BOR:
    aVal |= bVal;
    break;
  case BC_XOR:
    aVal ^= bVal;
    break;
  case BC_BLS:
    aVal = aVal << bVal;
    break;
  case BC_BRS:
    aVal = aVal >> bVal;
    break;
  default:
    return BC_NOT_IMPLEMENTED;
  }
  *result = bcValueInteger(aVal);
  return BC_OK;
}

static bcStatus_t bcValueBinaryOperator(const BC_VALUE a, const BC_VALUE b, uint8_t binop, BC_VALUE* result)
{
  switch (binop)
  {
  case BC_ADD:
  case BC_SUB:
  case BC_MUL:
  case BC_DIV:
  case BC_MOD:
    return bcValueBinaryOperatorAlgebra(a, b, binop, result);
  case BC_EQ:
  case BC_NEQ:
  case BC_GR:
  case BC_LS:
  case BC_GRE:
  case BC_LSE:
    return bcValueBinaryOperatorCompare(a, b, binop, result);
  case BC_LND:
  case BC_LOR:
  case BC_BND:
  case BC_BOR: 
  case BC_XOR:
  case BC_BLS:
  case BC_BRS:
    return bcValueBinaryOperatorLogicBitwise(a, b, binop, result);
  default:
    return BC_NOT_IMPLEMENTED;
  }
}

bcStatus_t bcCoreExecute(BC_CORE core, const char* code, char** endp)
{
  if ((core == NULL) || (code == NULL))
  {
    return BC_INVALID_ARG;
  }

  bcCodeStream_t codeStream;
  bcStatus_t result = bcParseString(code, &codeStream, endp);
  if (result != BC_OK)
  {
    return result;
  }

  for(const uint8_t* cursor = codeStream.opcodes, *end = codeStream.opcodes + codeStream.opSize; cursor != end; ++cursor)
  {
    switch (*cursor)
    {
    case BC_HALT:
      bcCodeStreamCleanup(&codeStream);
      return BC_OK;
    case BC_ADD:
    case BC_SUB:
    case BC_MUL:
    case BC_DIV:
    case BC_MOD:
    case BC_EQ:
    case BC_NEQ:
    case BC_GR:
    case BC_LS:
    case BC_GRE:
    case BC_LSE:
    case BC_LND:
    case BC_LOR:
    case BC_BND:
    case BC_BOR: 
    case BC_XOR:
    case BC_BLS:
    case BC_BRS:
      {
        if ((core->stack.top - core->stack.bottom) < 2)
        {
          return BC_UNDERFLOW;
        }

        BC_VALUE result;

        bcStatus_t status = bcValueBinaryOperator(
          core->stack.top[-2],
          core->stack.top[-1],
          *cursor,
          &result
        );

        if (status != BC_OK)
        {
          return status;
        }

        bcValueStackPop(&core->stack);
        bcValueStackPop(&core->stack);
        bcValueStackPush(&core->stack, result);
      }
      break;
    case BC_PSH:
      {
        ++cursor;
        if (cursor == end)
        {
          bcCodeStreamCleanup(&codeStream);
          return BC_MALFORMED_CODE;
        }

        uint8_t conID = *cursor;
        if (conID >= codeStream.conSize)
        {
          bcCodeStreamCleanup(&codeStream);
          return BC_CONST_NOT_FOUND;
        }

        BC_VALUE val = bcValueCopy(codeStream.cons[conID]);

        bcStatus_t status = bcValueStackPush(&core->stack, val);
        if (status != BC_OK)
        {
          return status;
        }
      }
      break;
    default:
      fprintf(stderr, "Unknown opcode: 0x%02X\n", *cursor);
      bcCodeStreamCleanup(&codeStream);
      return BC_NOT_IMPLEMENTED;
    }
  }

  assert(0);
  bcCodeStreamCleanup(&codeStream);
  return BC_HALT_EXPECTED;
}

BCAPI bcStatus_t bcCoreTop(const BC_CORE core, BC_VALUE* val)
{
  if ((core == NULL) || (val == NULL))
  {
    return BC_INVALID_ARG;
  }

  if (core->stack.top == core->stack.bottom)
  {
    return BC_UNDERFLOW;
  }

  *val = core->stack.top[-1];
  return BC_OK;
}

BCAPI bcStatus_t bcCorePop(BC_CORE core)
{
  if (core == NULL)
  {
    return BC_INVALID_ARG;
  }

  if (core->stack.top == core->stack.bottom)
  {
    return BC_UNDERFLOW;
  }

  bcValueCleanup(core->stack.top[-1]);
  --core->stack.top;
  return BC_OK;
}


bcStatus_t bcCodeStreamInit(bcCodeStream_t* cs)
{
  uint8_t* opcodes = (uint8_t*) calloc(BC_CODE_STREAM_INITIAL_OPCODE_CAP, sizeof(uint8_t));
  if (opcodes == NULL)
  {
    return BC_NO_MEMORY;
  }

  BC_VALUE* cons = (BC_VALUE*) calloc(BC_CODE_STREAM_INITIAL_CONST_CAP, sizeof(BC_VALUE));
  if (cons == NULL)
  {
    free(opcodes);
    return BC_NO_MEMORY;
  }

  cs->opcodes = opcodes;
  cs->opSize = 0;
  cs->opCap = BC_CODE_STREAM_INITIAL_OPCODE_CAP;

  cs->cons = cons;
  cs->conSize = 0;
  cs->conCap = BC_CODE_STREAM_INITIAL_CONST_CAP;
  return BC_OK;
}

bcStatus_t bcCodeStreamCleanup(bcCodeStream_t* cs)
{
  free(cs->opcodes);
  cs->opcodes = NULL;
  cs->opSize = 0;
  cs->opCap = 0;

  for (BC_VALUE* cursor = cs->cons, *end = cs->cons+cs->conSize;  cursor!=end; ++cursor)
  {
    bcValueCleanup(*cursor);
  }
  free(cs->cons);
  cs->cons = NULL;
  cs->conSize = 0;
  cs->conCap = 0;
  return BC_OK;
}

bcStatus_t bcCodeStreamAppendOpcode(bcCodeStream_t* cs, uint8_t opcode)
{
  if (cs == NULL) 
  {
    return BC_INVALID_ARG;
  }

  if (cs->opSize == cs->opCap)
  {
    uint8_t* newCodes = (uint8_t*) calloc(cs->opCap*3/2, sizeof(uint8_t));
    if (newCodes == NULL)
    {
      return BC_NO_MEMORY;
    }

    memcpy(newCodes, cs->opcodes, cs->opCap*sizeof(uint8_t));
    free(cs->opcodes);

    cs->opcodes = newCodes;
    cs->opCap = cs->opCap*3/2;
  }

  cs->opcodes[cs->opSize++] = opcode;
  return BC_OK;
}

bcStatus_t bcCodeStreamAppendConstant(bcCodeStream_t* cs, BC_VALUE con, uint8_t* pCon)
{
  if ((cs == NULL) || (pCon == NULL) || (con == NULL))
  {
    return BC_INVALID_ARG;
  }

  if (cs->conSize == (UINT8_MAX+1))
  {
    return BC_TOO_MANY_CONSTANTS;
  }

  if (cs->conSize == cs->conCap)
  {
    BC_VALUE* newCons = (BC_VALUE*) calloc(cs->conCap*3/2, sizeof(BC_VALUE));
    if (newCons == NULL)
    {
      return BC_NO_MEMORY;
    }

    memcpy(newCons, cs->cons, cs->conCap*sizeof(BC_VALUE));
    free(cs->cons);

    cs->cons = newCons;
    cs->conCap = cs->conCap*3/2;
  }

  *pCon = (uint8_t) (cs->conSize & 0xFF);
  cs->cons[cs->conSize++] = con;
  return BC_OK;  
}
