#ifndef __FIR_H__
#define __FIR_H__

#include "fir_func.h"

riscv_status riscv_fir_init_q15(
        riscv_fir_instance_q15 * S,
        uint16_t numTaps,
  const q15_t * pCoeffs,
        q15_t * pState,
        uint32_t blockSize);

void riscv_fir_q15(
  const riscv_fir_instance_q15 * S,
  const q15_t * pSrc,
        q15_t * pDst,
        uint32_t blockSize);
#endif