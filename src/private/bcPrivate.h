/**
 * @file bcPrivate.h
 * 
 * Private definitions for Bad Code Programming Language.
 * 
 */
#pragma once
#ifndef DECI_SPACE_BADCODE_PRIVATE_HEADER
#define DECI_SPACE_BADCODE_PRIVATE_HEADER

#include <badcode.h>
#include "bcValue.h"
#include "bcValueStack.h"

/**
 * Maximum stack size. Interpreter exits with BC_OVERFLOW if stack exceed this 
 * size.
 */
#define BC_CORE_VALUE_STACK_SIZE (4096)

/**
 * Initial code stream opcode capacity. It increases using CAP1 = CAP*3/2
 * formula when actual size exceeds current capacity, where CAP1 - new capacity,
 * CAP - old capacity.
 */
#define BC_CODE_STREAM_INITIAL_OPCODE_CAP (16)

/**
 * Initial code stream constants capacity. It increases using CAP1 = CAP*3/2 
 * formula when actual size exceeds current capacity, where CAP1 - new capacity,
 * CAP - old capacity
 */
#define BC_CODE_STREAM_INITIAL_CONST_CAP (2)

/**
 * Interpreter bytecodes.
 * 
 * Only few bytecodes has additional arguments passed after it.
 * 
 * As an example: after BC_PSH follows byte encoding constant ID to push.
 */
typedef enum bcOp_t
{
  BC_HALT = 0x00, /**< Halt VM Execution */
  BC_ADD, /**< A + B */
  BC_SUB, /**< A - B */
  BC_MUL, /**< A * B */
  BC_DIV, /**< A / B */
  BC_MOD, /**< A % B */
  BC_EQ,  /**< A = B */
  BC_NEQ, /**< A != B */
  BC_GR,  /**< A > B */
  BC_LS,  /**< A < B */
  BC_GRE, /**< A >= B */
  BC_LSE, /**< A <= B */
  BC_LND, /**< A && B */
  BC_LOR, /**< A || B */
  BC_BND, /**< A & B */
  BC_BOR, /**< A | B */ 
  BC_XOR, /**< A ^ B */
  BC_BLS, /**< A << B */
  BC_BRS, /**< A >> B */
  BC_IND, /**< A[B] */
  BC_ADR, /**< &A */
  BC_ITM, /**< A.B */
  BC_CLL, /**< A() */
  BC_NEG, /**< -A */
  BC_INC, /**< ++A */
  BC_DEC, /**< --A */
  BC_LNT, /**< !A */
  BC_BNT, /**< ~A */
  BC_INT, /**< toInteger(A) */
  BC_NUM, /**< toNumber(A) */
  BC_STR, /**< toString(A) */
  BC_LST, /**< toList(A) */
  BC_DCT, /**< toDict(A) */
  BC_SET, /**< A <- B */
  BC_PSH, /**< push(A) */
  BC_POP, /**< pop()   */
  BC_OP_LAST, /**< Last valid opcode */
  BC_OP_TOTAL = 0xFF
} bcOp_t;

/**
 * Abstraction for chunk of compiled code without branches.
 */
typedef struct bcCodeStream_t
{
  size_t opCap;     /**< Total opcode capacity */
  size_t opSize;    /**< Total opcode size     */
  uint8_t* opcodes; /**< opcodes               */

  size_t    conCap;  /**< Total consts capacity */
  size_t    conSize; /**< Total consts size     */
  BC_VALUE* cons;    /**< Constants             */
} bcCodeStream_t;

/**
 * Interprerer evaluation core.
 */
struct bcCore_t
{
  bcValueStack_t stack;
};

/**
 * Initialize code stream in-place.
 * 
 * Function allocates space for bcCodeStream_t. Structure must not be previosly
 * initialized, or memory leak happens.
 * 
 * If function failed, no changes to data passed in argument are applied.
 * 
 * @param cs[in] pointer to uninitialized code stream
 * 
 * @return 
 *  BC_OK - if code stream initialized successfully
 *  BC_NO_MEMORY - if some data allocation failed.
 */
bcStatus_t bcCodeStreamInit(bcCodeStream_t* cs);

/**
 * Cleanup valid code stream.
 * 
 * Function cleanup memory for valid code stream, frees all stored inside BC_VALUEs.
 * 
 * @param cs[in] pointer to valid code stream
 * 
 * @return
 *   BC_OK - always, may change later
 */
bcStatus_t bcCodeStreamCleanup(bcCodeStream_t* cs);

/**
 * Appends new opcode at end of opcode list.
 * 
 * When opcode capacity is reached, allocates new opcode array of bigger size
 * copy all opcodes from old array to new and frees old array.
 * 
 * No checks for opcode validity are made in this function.
 * 
 * @param cs[in] - valid code stream
 * @param opcode[in] - opcode to append
 * 
 * @return
 *    BC_INVALID_ARG - if passed cs == NULL
 *    BC_NO_MEMORY - when capacity reached and no memory can't be allocated
 *    BC_OK - opcode appended successfully
 */
bcStatus_t bcCodeStreamAppendOpcode(bcCodeStream_t* cs, uint8_t opcode);

/**
 * Appends new constant at end of constant list and returns it's ID.
 * 
 * When constant capacity is reached, allocates new constant array of bigger size
 * copy all constants from old array to new and frees old array.
 * 
 * Function check constant pointer equality to NULL. Code stream owns passed
 * constant's BC_VALUE if function call completed successfully.
 * 
 * @param cs[in] - valid code stream
 * @param con[in] - constant to add
 * @param pCon[out] - pointer to store new constant ID.
 * 
 * @return 
 *    BC_INVALID_ARG - if cs == NULL, or pCon == NULL, or con == NULL
 *    BC_TOO_MANY_CONSTANTS - total count of constants can't exceeds UINT8_MAX+1
 *    BC_NO_MEMORY - when capactity reached and no memory can't be allocated
 *    BC_OK - constant appened successfully
 */
bcStatus_t bcCodeStreamAppendConstant(bcCodeStream_t* cs, BC_VALUE con, uint8_t* pCon);

/**
 * Interface function to re2c generated lexer.
 * 
 * Split input character stream into tokens.
 * 
 * @param[in] head - first character in stream
 * @param[out] tail - pointer is set to character after last processed
 * @param[out] pData - pointer is set to new allocated BC_VALUE, if any.
 * 
 * @return 0 if string ended and no tokens are found, one of TOK_ constants otherwise
 */
int bcGetToken(const char* head, const char** tail, BC_VALUE* pData);

/**
 * Interface function to LEMON generated parser.
 * 
 * Generate code stream from given character string.
 * 
 * Code stream is initialized inside function, so only uninitialized streams must
 * be passed, or memory leak happens.
 * 
 * @param[in] str - string to parse into code stream
 * @param[out] codeStream - pointer to code stream to fill, if function fails, no changes to codeStream are made.
 * @param[out] endp - pointer in str to last processed character without error.
 * 
 * @return 
 *    BC_INVALID_ARG - when str or codeStream equals to NULL
 *    BC_NO_MEMORY - when code stream allocation fails, or parser failed to get memory
 *    BC_OK - if parsing completed successfully
 */
bcStatus_t bcParseString(const char* str, bcCodeStream_t* codeStream, char** endp);

#endif /* DECI_SPACE_BADCODE_PRIVATE_HEADER */
