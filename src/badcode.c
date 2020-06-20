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

int bcCompareGlobals(const void* a,const void* b)
{
  const BC_GLOBAL* aVal = (const BC_GLOBAL*)a;
  const BC_GLOBAL* bVal = (const BC_GLOBAL*)b;
  return strcmp((*aVal)->name, (*bVal)->name);
}

static BC_GLOBAL bcGlobalNew(const char* name, const BC_VALUE value)
{
  assert(name != NULL);

  size_t nameLen = strlen(name) + 1;

  BC_GLOBAL result = (BC_GLOBAL) malloc(sizeof(bcGlobalVar_t)+nameLen);
  if (result == NULL)
  {
    return NULL;
  }

  result->value = bcValueCopy(value);
  memcpy(result->name, name, nameLen);
  return result;
}

static void bcGlobalDelete(BC_GLOBAL global)
{
  if(global != NULL)
  {
    bcValueCleanup(global->value);
    free(global);
  }
}

bcStatus_t bcCoreSetGlobal(BC_CORE core, const char* name, const BC_VALUE value)
{
  BC_GLOBAL newGlobalVal = bcGlobalNew(name, value);
  if (newGlobalVal == NULL)
  {
    return BC_NO_MEMORY;
  }

  BC_GLOBAL* itemInArray = bsearch(&newGlobalVal, core->globals, core->globalSize, sizeof(BC_GLOBAL), bcCompareGlobals);
  if (itemInArray != NULL)
  {
    bcGlobalDelete(*itemInArray);
    *itemInArray = newGlobalVal;
    return BC_OK;
  }

  if (core->globalSize == core->globalCap)
  {
    BC_GLOBAL* newGlobals = (BC_GLOBAL*) calloc(core->globalCap*3/2, sizeof(BC_GLOBAL));
    if (newGlobals == NULL)
    {
      bcGlobalDelete(newGlobalVal);
      return BC_NO_MEMORY;
    }

    memcpy(newGlobals, core->globals, core->globalCap*sizeof(BC_GLOBAL));
    free(core->globals);

    core->globals = newGlobals;
    core->globalCap = core->globalCap*3/2;
  }

  core->globals[core->globalSize++] = newGlobalVal;
  qsort(core->globals, core->globalSize, sizeof(BC_GLOBAL), bcCompareGlobals);
  return BC_OK;
}

static BC_VALUE bcCoreGetGlobal(BC_CORE core, const char* name)
{
  BC_GLOBAL keyGlobal = bcGlobalNew(name, NULL);
  if (keyGlobal == NULL)
  {
    return NULL;
  }

  BC_GLOBAL* itemInArray = bsearch(&keyGlobal, core->globals, core->globalSize, sizeof(BC_GLOBAL), bcCompareGlobals);
  if (itemInArray != NULL)
  {
    bcGlobalDelete(keyGlobal);
    return bcValueCopy((*itemInArray)->value);
  }
  bcGlobalDelete(keyGlobal);
  return NULL;
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

  result->globalCap = BC_CORE_GLOBAL_INITIAL_CAP;
  result->globalSize = 0;
  result->globals = (BC_GLOBAL*) calloc(BC_CORE_GLOBAL_INITIAL_CAP, sizeof(BC_GLOBAL));
  if (result->globals == NULL)
  {
    bcValueStackCleanup(&result->stack);
    free(result);
    return BC_NO_MEMORY;
  }

  *pCore = result;
  return BC_OK;
}

void bcCoreDelete(BC_CORE core)
{
  if (core != NULL)
  {
    for (BC_GLOBAL* cursor = core->globals, *end = core->globals + core->globalSize; cursor != end; ++cursor)
    {
      bcGlobalDelete(*cursor);
    }
    free(core->globals);

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
          if (bVal == 0)
          {
            return BC_DIVIDE_BY_ZERO;
          }
          aVal /= bVal;
          break;
        case BC_MOD:
          if (bVal == 0)
          {
            return BC_DIVIDE_BY_ZERO;
          }
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

static bcStatus_t bcValueUnaryOperator(const BC_VALUE a, uint8_t unop, BC_VALUE* result)
{
  assert((result != NULL) && (a != NULL));

  switch (unop)
  {
  case BC_NEG:
    switch (a->type)
    {
    case BC_INTEGER:
      {
        const bcInteger_t* aVal = (const bcInteger_t*)a;
        *result = bcValueInteger(-aVal->data);
        return BC_OK;
      }
      break;
    case BC_NUMBER:
      {
        const bcNumber_t* aVal = (const bcNumber_t*)a;
        *result = bcValueNumber(-aVal->data);
        return BC_OK;
      }
      break;
    default:
      return BC_NOT_IMPLEMENTED;
    }
    break;
  case BC_LNT:
  case BC_BNT:
    {
      if (a->type != BC_INTEGER)
      {
        return BC_NOT_IMPLEMENTED;
      }
      const bcInteger_t* aVal = (const bcInteger_t*)a;
      if (unop == BC_LNT)
      {
        *result = bcValueInteger(!aVal->data);
      }
      else
      {
        *result = bcValueInteger(~aVal->data);
      }
      return BC_OK;
    }
    break;
  case BC_INT:
    {
      int64_t aVal;
      bcStatus_t status = bcValueAsInteger(a, &aVal);
      if (status != BC_OK)
      {
        return status;
      }
      *result = bcValueInteger(aVal);
      return BC_OK;
    }
    break;
  case BC_NUM:
    {
      double aVal;
      bcStatus_t status = bcValueAsNumber(a, &aVal);
      if (status != BC_OK)
      {
        return status;
      }
      *result = bcValueNumber(aVal);
      return BC_OK;
    }
    break;
  case BC_STR:
    {
      char* aVal = NULL;
      bcStatus_t status = bcValueAsString(a, &aVal, 0);
      if (status != BC_OK)
      {
        return status;
      }
      *result = bcValueString(aVal);
      free(aVal);
      return BC_OK;
    }
    break;
  default:
    return BC_NOT_IMPLEMENTED;
  }
}

static const char* bcOpcodeString(uint8_t opcode)
{
  switch (opcode)
  {
  case BC_HALT: return "HALT";/**< Halt VM Execution */
  case BC_PSH: return "PSH"; /**< push(A) */
  case BC_POP: return "POP"; /**< pop() */
  case BC_ADD: return "ADD"; /**< A + B */
  case BC_SUB: return "SUB"; /**< A - B */
  case BC_MUL: return "MUL"; /**< A * B */
  case BC_DIV: return "DIV"; /**< A / B */
  case BC_MOD: return "MOD"; /**< A % B */
  case BC_EQ: return "EQ";  /**< A = B */
  case BC_NEQ: return "NEQ"; /**< A != B */
  case BC_GR: return "GR";  /**< A > B */
  case BC_LS: return "LS";  /**< A < B */
  case BC_GRE: return "GRE"; /**< A >= B */
  case BC_LSE: return "LSE"; /**< A <= B */
  case BC_LND: return "LND"; /**< A && B */
  case BC_LOR: return "LOR"; /**< A || B */
  case BC_BND: return "BND"; /**< A & B */
  case BC_BOR: return "BOR"; /**< A | B */ 
  case BC_XOR: return "XOR"; /**< A ^ B */
  case BC_BLS: return "BLS"; /**< A << B */
  case BC_BRS: return "BRS"; /**< A >> B */
  case BC_NEG: return "NEG"; /**< -A */
  case BC_LNT: return "LNT"; /**< !A */
  case BC_BNT: return "BNT"; /**< ~A */
  case BC_INT: return "INT"; /**< (int) A */
  case BC_NUM: return "NUM"; /**< (num) A */
  case BC_STR: return "STR"; /**< (str) A */
  case BC_SET: return "SET"; /**< A <- B */
  case BC_VAL: return "VAL"; /**< ValueOf(A) */
  case BC_IND: return "IND"; /**< A[B] */
  case BC_ADR: return "ADR"; /**< &A */
  case BC_ITM: return "ITM"; /**< A.B */
  case BC_CLL: return "CLL"; /**< A() */
  case BC_LST: return "LST"; /**< toList(A) */
  case BC_DCT: return "DCT"; /**< toDict(A) */
  default:
    assert(0);
    return "???";
  }
}

bcStatus_t bcCoreExecute(BC_CORE core, const char* code, char** endp)
{
  #define BC_CORE_RETURN(STATUS) coreResult = (STATUS); goto CORE_EXIT

  if ((core == NULL) || (code == NULL))
  {
    return BC_INVALID_ARG;
  }

  bcCodeStream_t codeStream;
  bcStatus_t coreResult = bcParseString(code, &codeStream, endp);
  if (coreResult != BC_OK)
  {
    return coreResult;
  }

  for(const uint8_t* cursor = codeStream.opcodes, *end = codeStream.opcodes + codeStream.opSize; cursor != end; ++cursor)
  {
    fprintf(stderr, "%s\n", bcOpcodeString(*cursor));
    switch (*cursor)
    {
    case BC_HALT:
      BC_CORE_RETURN(BC_OK);
    case BC_PSH:
      {
        ++cursor;
        if (cursor == end)
        {
          BC_CORE_RETURN(BC_MALFORMED_CODE);
        }

        uint8_t conID = *cursor;
        if (conID >= codeStream.conSize)
        {
          BC_CORE_RETURN(BC_CONST_NOT_FOUND);
        }

        bcStatus_t status = bcValueStackPush(&core->stack, codeStream.cons[conID]);
        if (status != BC_OK)
        {
          BC_CORE_RETURN(status);
        }
      }
      break;
    case BC_POP:
      {
        bcStatus_t status = bcValueStackPop(&core->stack);
        if (status != BC_OK)
        {
          BC_CORE_RETURN(status);
        }
      }
      break;
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
          BC_CORE_RETURN(BC_UNDERFLOW);
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
          BC_CORE_RETURN(status);
        }

        bcValueStackPop(&core->stack);
        bcValueStackPop(&core->stack);
        bcValueStackPush(&core->stack, result);
        bcValueCleanup(result);
      }
      break;
    case BC_SET:
      {
        if ((core->stack.top - core->stack.bottom) < 2)
        {
          BC_CORE_RETURN(BC_UNDERFLOW);
        }

        BC_VALUE id = core->stack.top[-2];
        if (id->type != BC_STRING)
        {
          BC_CORE_RETURN(BC_INVALID_ID);
        }

        BC_VALUE result = bcValueCopy(core->stack.top[-1]);

        bcStatus_t status = bcCoreSetGlobal(core, ((bcString_t*)id)->data, core->stack.top[-1]);
        if (status != BC_OK)
        {
          BC_CORE_RETURN(status);
        }
        bcValueStackPop(&core->stack);
        bcValueStackPop(&core->stack);
        bcValueStackPush(&core->stack, result);
        bcValueCleanup(result);
      }
      break;
    case BC_NEG:
    case BC_LNT:
    case BC_BNT:
    case BC_INT:
    case BC_NUM:
    case BC_STR:
      {
        if ((core->stack.top - core->stack.bottom) < 1)
        {
          BC_CORE_RETURN(BC_UNDERFLOW);
        }

        BC_VALUE result;

        bcStatus_t status = bcValueUnaryOperator(
          core->stack.top[-1],
          *cursor,
          &result
        );

        if (status != BC_OK)
        {
          BC_CORE_RETURN(status);
        }

        bcValueStackPop(&core->stack);
        bcValueStackPush(&core->stack, result);
        bcValueCleanup(result);
      }
      break;
    case BC_VAL:
      {
        if ((core->stack.top - core->stack.bottom) < 1)
        {
          BC_CORE_RETURN(BC_UNDERFLOW);
        }

        BC_VALUE id = core->stack.top[-1];
        if (id->type != BC_STRING)
        {
          BC_CORE_RETURN(BC_INVALID_ID);
        }

        BC_VALUE result = bcCoreGetGlobal(core, ((bcString_t*)id)->data);
        if (result == NULL)
        {
          BC_CORE_RETURN(BC_NOT_DEFINED);
        }

        bcValueStackPop(&core->stack);
        bcValueStackPush(&core->stack, result);
        bcValueCleanup(result);
      }
      break;
    default:
      fprintf(stderr, "Unknown opcode: 0x%02X\n", *cursor);
      bcCodeStreamCleanup(&codeStream);
      BC_CORE_RETURN(BC_NOT_IMPLEMENTED);
    }
  }

  assert(0);
  BC_CORE_RETURN(BC_HALT_EXPECTED);

  #undef BC_CORE_RETURN
CORE_EXIT:

  bcCodeStreamCleanup(&codeStream);
  return coreResult;
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

bcStatus_t bcCodeStreamAppendConstant(bcCodeStream_t* cs, const BC_VALUE con, uint8_t* pCon)
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
  cs->cons[cs->conSize++] = bcValueCopy(con);
  return BC_OK;  
}
