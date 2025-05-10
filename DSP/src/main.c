#include "fir.h"

#define INPUT_SIZE 320
#define NUM_TAPS 29
#define BLOCK_SIZE 32
#define BLOCK_NUM (INPUT_SIZE / BLOCK_SIZE)

const q15_t firCoeffs32[NUM_TAPS] = {
    1, 2, 3, 1, 2, 3, 2, 2,
    4, 2, 3, 1, 4, 4, 0, 2,
    0, 2, 3, 4, 2, 1, 3, 3,
    1, 1, 4, 2, 4,
};

static q15_t testOutput[INPUT_SIZE];

extern q15_t testInput_f32_1kHz_15kHz[INPUT_SIZE];
extern 

int main() {
    riscv_fir_instance_q15 S;
    // riscv_status status;
    q15_t *input = testInput_f32_1kHz_15kHz;
    q15_t *output = testOutput;
    

    riscv_fir_init_q15(&S, NUM_TAPS, firCoeffs32, NULL, BLOCK_SIZE);
    for (int i = 0; i < BLOCK_NUM; i++) {
        riscv_fir_q15(&S, input, output, BLOCK_SIZE);
    }
    return 0;

}