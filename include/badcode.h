/**
 * @file badcode.h
 * @author masscry
 * 
 * Bad Code Programming Language Public Header.
 */

#pragma once
#ifndef DECI_SPACE_BADCODE_HEADER
#define DECI_SAPCE_BADCODE_HEADER

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#define BCAPI extern "C"
#else
#define BCAPI
#endif

/**
 * Full version of BadCode library
 */
#define BC_VER_FULL (0x00010000)

/**
 * Major part of BadCode library version
 */
#define BC_VER_MAJOR (0x00)

/**
 * Minor part of BadCode library version
 */
#define BC_VER_MINOR (0x01)

/**
 * Patch part of BadCode library version
 */
#define BC_VER_PATCH (0x0000)

/**
 * BadcCode library version string.
 */
#define BC_VER_STRING ("0.1.0")

/**
 * Main evaluation engine.
 */
typedef struct bcCore_t* BC_CORE;

/**
 * Simple box structure implements "VARIANT" pattern
 */
typedef struct bcValue_t* BC_VALUE;

/**
 * Status codes.
 */
typedef enum bcStatus_t
{
  BC_OK = 0,             /**< No Errors */
  BC_INVALID_ARG,        /**< Argument check failed */
  BC_NOT_IMPLEMENTED,    /**< Required command is not implemented (?yet) */
  BC_NO_MEMORY,          /**< Memory allocation failed */
  BC_TOO_MANY_CONSTANTS, /**< There are too many constants in interpreted source code */
  BC_UNDERFLOW,          /**< Not enough elements on stack */
  BC_OVERFLOW,           /**< There are too many elements on stack */
  BC_MALFORMED_CODE,     /**< Invalid opcode */
  BC_CONST_NOT_FOUND,    /**< Requested constant is not found */
  BC_HALT_EXPECTED,      /**< Program end reached before HALT */
  BC_STATUS_TOTAL        /**< Total status codes */
} bcStatus_t;

/**
 * Version of loaded BadCode library.
 */
BCAPI uint32_t bcVersion();

/**
 * Create new BadCode core instance.
 * 
 * @return NULL on errors, new BC_CORE otherwise.
 */
bcStatus_t bcCoreNew(BC_CORE* pCore);

/**
 * Delete BadCode core instance.
 * 
 * If NULL pointer is passed as core, function has no effect.
 * 
 * @param core[in] core to delete
 * 
 */
BCAPI void bcCoreDelete(BC_CORE core);

/**
 * Execute code on given core.
 * 
 * @param core[in] valid core to execute code on
 * @param code[in] string of code to execute
 * @param endp[out,opt] if not NULL, set to last valid executed command
 * 
 * @return BC_OK if execution completed successfully, error code otherwise.
 */
BCAPI bcStatus_t bcCoreExecute(BC_CORE core, const char* code, char** endp);

/**
 * Get value on top of stack.
 */
BCAPI bcStatus_t bcCoreTop(const BC_CORE core, BC_VALUE* pVal);

/**
 * Box an integer.
 * 
 * @param[in] val value to box in BC_VALUE
 * 
 * @return NULL on errors, new value otherwise
 */
BCAPI BC_VALUE bcValueInteger(int64_t val);

/**
 * Create copy of given box.
 * 
 * @param[in] val valid value box
 * 
 * @return NULL on errors, new copy otherwise
 */
BCAPI BC_VALUE bcValueCopy(const BC_VALUE val);

/**
 * Free value box.
 * 
 * @param[in] value valid box to cleanup.
 */
BCAPI bcStatus_t bcValueCleanup(BC_VALUE value);

/**
 * Get stored value as integer.
 * 
 * @param[in] val valid BC_VALUE
 * @param[out] oval pointer to store boxed value
 * 
 * @return BC_OK if completed sucessfuly, error code otherwise
 */
BCAPI bcStatus_t bcValueAsInteger(const BC_VALUE val, int64_t* oval);

/**
 * Box a number.
 * 
 * @param[in] val value to box in BC_VALUE
 * 
 * @return NULL on errors, new value otherwise
 */
BCAPI BC_VALUE bcValueNumber(double val);

/**
 * Get stored value as number.
 * 
 * @param[in] val valid BC_VALUE
 * @param[out] oval pointer to store boxed value
 * 
 * @return BC_OK if completed sucessfuly, error code otherwise
 */
BCAPI bcStatus_t bcValueAsNumber(const BC_VALUE val, double* oval);

#endif /* DECI_SPACE_BADCODE_HEADER */