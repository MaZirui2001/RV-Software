#ifndef __MATH_TYPE_H__
#define __MATH_TYPE_H__

#include "stdint.h"
typedef int16_t q15_t;
typedef int32_t q31_t;
typedef int64_t q63_t;

typedef enum
{
  RISCV_MATH_SUCCESS                 =  0,        /**< No error */
  RISCV_MATH_ARGUMENT_ERROR          = -1,        /**< One or more arguments are incorrect */
  RISCV_MATH_LENGTH_ERROR            = -2,        /**< Length of data buffer is incorrect */
  RISCV_MATH_SIZE_MISMATCH           = -3,        /**< Size of matrices is not compatible with the operation */
  RISCV_MATH_NANINF                  = -4,        /**< Not-a-number (NaN) or infinity is generated */
  RISCV_MATH_SINGULAR                = -5,        /**< Input matrix is singular and cannot be inverted */
  RISCV_MATH_TEST_FAILURE            = -6,        /**< Test Failed */
  RISCV_MATH_DECOMPOSITION_FAILURE   = -7         /**< Decomposition Failed */
} riscv_status;
#endif
