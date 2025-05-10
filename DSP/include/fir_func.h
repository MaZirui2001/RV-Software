#ifndef __FIR_FUNC_H__
#define __FIR_FUNC_H__

#include "math_type.h"
#include "compatitable.h"
#include "common.h"
typedef struct
{
        uint16_t numTaps;         /**< number of filter coefficients in the filter. */
        q15_t *pState;            /**< points to the state variable array. The array is of length numTaps+blockSize-1. */
  const q15_t *pCoeffs;           /**< points to the coefficient array. The array is of length numTaps.*/
} riscv_fir_instance_q15;

#endif
