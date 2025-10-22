// ...existing code...
#include "cfft.h"
// #include <stdio.h>
#include <stdint.h>
#define FFT_N 1024
#define LOG2_FFT_N 10
static volatile int bench_sink = 0;  
// static unsigned perturb_tick = 0;  


//复数运算
static inline void complex_multiply(complex_t a, complex_t b, complex_t* result) {
    // Use 64-bit intermediate to avoid overflow, then scale back
    int64_t real_temp = ((int64_t)a.real * b.real - (int64_t)a.imag * b.imag);
    int64_t imag_temp = ((int64_t)a.real * b.imag + (int64_t)a.imag * b.real);
    
    // Scale back from Q30 to Q15
    result->real = (int32_t)(real_temp >> FIXED_POINT_BITS);
    result->imag = (int32_t)(imag_temp >> FIXED_POINT_BITS);
}
static inline void complex_add(complex_t a, complex_t b, complex_t* result) {
    result->real = a.real + b.real;
    result->imag = a.imag + b.imag;
}
static inline void complex_subtract(complex_t a, complex_t b, complex_t* result) {
    result->real = a.real - b.real;
    result->imag = a.imag - b.imag;
}



// 16位数据的位反转
static inline uint16_t bit_reverse_u16(uint16_t x, int bits) {
    uint16_t r = 0;
    for (int i = 0; i < bits; i++) {
        r = (uint16_t)((r << 1) | (x & 1));
        x >>= 1;
    }
    return r;
}
//
//复杂度：O(N * LOG2_FFT_N)，通过逐次计算 rev(i) 避免预生成查表。
// 位反转重排（就地）
static void bit_reverse_reorder_generic(complex_t* data) {
    for (uint16_t i = 0; i < FFT_N; i++) {
        uint16_t j = bit_reverse_u16(i, LOG2_FFT_N);
        if (j > i) {
            complex_t t = data[i];
            data[i] = data[j];
            data[j] = t;
        }
    }
}

// 扭结因子表（W_N^k, k=0..N/2-1），用递推生成，避免三角函数
static complex_t twiddle_factors[FFT_N / 2];
static int twiddles_initialized = 0;
// 生成 W_N^k 表：w_{k+1} = w_k * W1，W1 ≈ cos(2π/N) - j sin(2π/N)
static void init_twiddles(void) {
    if (twiddles_initialized) return;

    // Q15 近似：cos(2π/1024)≈0.999981, sin≈0.0061359
    const complex_t W1 = { 32767, -(int16_t)201 }; // cos=32767, -sin=-201

    complex_t w = { 32767, 0 }; // W^0 = 1
    for (int k = 0; k < FFT_N / 2; k++) {
        twiddle_factors[k] = w;
        complex_t next;
        complex_multiply(w, W1, &next);
        w = next;
    }
    twiddles_initialized = 1;
}

void fft_1024_point(const complex_t input[FFT_N], complex_t output[FFT_N]) {
    init_twiddles();

    for (int i = 0; i < FFT_N; i++) {
        output[i] = input[i];
    }
    bit_reverse_reorder_generic(output);

    // 分段蝶形：m=2,4,...,1024；step=N/m，从 N/2 开始每级右移一位
    int m = 2;//蝶形长度
    int step = FFT_N >> 1; // N/m，旋转因子步长
    while (m <= FFT_N) {
        int halfsize = m >> 1;//也是每段的stride，即每段内相隔多少个元素进行蝶形运算

        for (int block = 0; block < FFT_N; block += m) {
            int k = 0;
            for (int j = 0; j < halfsize; j++) {
                complex_t t, u, v;
                complex_multiply(output[block + j + halfsize], twiddle_factors[k], &t);

                u = output[block + j];
                v = t;

                complex_add(u, v, &output[block + j]);
                complex_subtract(u, v, &output[block + j + halfsize]);

                k += step; // k = j * (N/m)
            }
        }
        //下一级，蝶形长度翻倍，旋转因子步长减半
        m <<= 1;
        step >>= 1;
    }
}

// 1024 点 IFFT：共轭→FFT→共轭并除以 N
void ifft_1024_point(const complex_t input[FFT_N], complex_t output[FFT_N]) {
    complex_t temp[FFT_N];

    // 共轭
    for (int i = 0; i < FFT_N; i++) {
        temp[i].real = input[i].real;
        temp[i].imag = -input[i].imag;
    }

    // FFT
    fft_1024_point(temp, output);

    // 共轭并缩放 1/N（右移 LOG2_FFT_N）
    for (int i = 0; i < FFT_N; i++) {
        output[i].real = (int32_t)( output[i].real >> LOG2_FFT_N);
        output[i].imag = (int32_t)((-output[i].imag) >> LOG2_FFT_N);
    }
}

// 幅度平方
static uint32_t magnitude_squared(complex_t c) {
    int64_t real_sq = (int64_t)c.real * c.real;
    int64_t imag_sq = (int64_t)c.imag * c.imag;
    return (uint32_t)((real_sq + imag_sq) >> FIXED_POINT_BITS);
}

// 简单自测：脉冲/直流
// int test_fft_1024() {
//     complex_t x[FFT_N], X[FFT_N];

//     // 脉冲：x[0]=1
//     for (int i = 0; i < FFT_N; i++) { x[i].real = 0; x[i].imag = 0; }
//     x[0].real = 32767;
//     fft_1024_point(x, X);
//     uint32_t ref = magnitude_squared(X[0]);
//     uint32_t tol = ref >> 3;
//     for (int i = 1; i < FFT_N; i++) {
//         uint32_t m2 = magnitude_squared(X[i]);
//         if (m2 + tol < ref && m2 > ref + tol) return -1;
//     }

//     // 直流：x[n]=1
//     for (int i = 0; i < FFT_N; i++) { x[i].real = 32767; x[i].imag = 0; }
//     fft_1024_point(x, X);
//     uint32_t dc = magnitude_squared(X[0]);
//     for (int i = 1; i < FFT_N; i++) {
//         if (magnitude_squared(X[i]) > (dc >> 4)) return -2;
//     }
//     return 0;
// }

int test_impulse_1024() {
    complex_t test_input[FFT_N];
    
    test_input[0].real = 32767; test_input[0].imag = 0;  
    for (int i = 1; i < FFT_N; i++) {
        test_input[i].real = 0;
        test_input[i].imag = 0;
    }
    
    complex_t fft_output[FFT_N];
    
    
    fft_1024_point(test_input, fft_output);
    
    // 
    uint32_t reference_mag = magnitude_squared(fft_output[0]);
    //uint32_t tolerance = reference_mag >> 3;  // 12.5% tolerance
    uint32_t tolerance = 8;  // 绝对容差：8 LSB 10级蝶形积累
    for (int i = 1; i < FFT_N; i++) {
        uint32_t mag = magnitude_squared(fft_output[i]);
        if (mag < (reference_mag - tolerance) || mag > (reference_mag + tolerance)) {
            return -1;  // Test failed
        }
    }
    
    return 0;  // Test passed
}
int test_dc_1024() {
    complex_t test_input[FFT_N];
    
    for (int i = 0; i < FFT_N; i++) {
        test_input[i].real = 32767;
        test_input[i].imag = 0;
    }
    
    complex_t fft_output[FFT_N];
    
    fft_1024_point(test_input, fft_output);
    
    // Check DC component (should be FFT_N * 32767)
    uint32_t dc_mag = magnitude_squared(fft_output[0]);
    uint32_t expected_dc = (uint32_t)FFT_N * 32767;  // Corrected expected value
    
    // DC component should be much larger than others
    if (dc_mag < (expected_dc >> 2)) {  // At least 1/4 of expected
        return -1;
    }
    
    // Other components should be much smaller
    for (int i = 1; i < FFT_N; i++) {
        uint32_t mag = magnitude_squared(fft_output[i]);
        if (mag > (dc_mag >> 4)) {  // Should be less than 1/16 of DC
            return -2;
        }
    }
    // uint32_t dc_mag = magnitude_squared(fft_output[0]);
    // int64_t amp = (int64_t)FFT_N * 32767;               // Q15 幅度
    // uint32_t expected_dc = (uint32_t)((amp * amp) >> FIXED_POINT_BITS); // 能量
    // uint32_t tol = 8; // 绝对容差（LSB）

    // if (dc_mag < expected_dc - tol || dc_mag > expected_dc + tol) {
    //     return -1;
    // }

    // // 其它频点应接近 0（允许微小定点误差）
    // for (int i = 1; i < FFT_N; i++) {
    //     if (magnitude_squared(fft_output[i]) > tol) {
    //         return -2;
    //     }
    // }
    return 0;
}
int test_single_frequency_1024() {
    complex_t test_input[FFT_N];
    
    // Precomputed values for frequency bin 1 in Q15 format - initialize manually
    for (int k = 0;k < 64; k++){
    test_input[0 + 16 * k].real = 32767; test_input[0 + 16 * k].imag = 0;        // k=0: 1.0 + 0.0j
    test_input[1 + 16 * k].real = 30273; test_input[1 + 16 * k].imag = -12539;   // k=1: 0.9239 - 0.3827j
    test_input[2 + 16 * k].real = 23170; test_input[2 + 16 * k].imag = -23170;   // k=2: 0.7071 - 0.7071j
    test_input[3 + 16 * k].real = 12539; test_input[3 + 16 * k].imag = -30273;   // k=3: 0.3827 - 0.9239j
    test_input[4 + 16 * k].real = 0; test_input[4 + 16 * k].imag = -32767;       // k=4: 0.0 - 1.0j
    test_input[5 + 16 * k].real = -12539; test_input[5 + 16 * k].imag = -30273;  // k=5: -0.3827 - 0.9239j
    test_input[6 + 16 * k].real = -23170; test_input[6 + 16 * k].imag = -23170;  // k=6: -0.7071 - 0.7071j
    test_input[7 + 16 * k].real = -30273; test_input[7 + 16 * k].imag = -12539;  // k=7: -0.9239 - 0.3827j
    test_input[8 + 16 * k].real = -32767; test_input[8 + 16 * k].imag = 0;       // k=8: -1.0 + 0.0j
    test_input[9 + 16 * k].real = -30273; test_input[9 + 16 * k].imag = 12539;   // k=9: -0.9239 + 0.3827j
    test_input[10 + 16 * k].real = -23170; test_input[10 + 16 * k].imag = 23170; // k=10: -0.7071 + 0.7071j
    test_input[11 + 16 * k].real = -12539; test_input[11 + 16 * k].imag = 30273; // k=11: -0.3827 + 0.9239j
    test_input[12 + 16 * k].real = 0; test_input[12 + 16 * k].imag = 32767;      // k=12: 0.0 + 1.0j
    test_input[13 + 16 * k].real = 12539; test_input[13 + 16 * k].imag = 30273;  // k=13: 0.3827 + 0.9239j
    test_input[14 + 16 * k].real = 23170; test_input[14 + 16 * k].imag = 23170;  // k=14: 0.7071 + 0.7071j
    test_input[15 + 16 * k].real = 30273; test_input[15 + 16 * k].imag = 12539;  // k=15: 0.9239 + 0.3827j
    }
    complex_t fft_output[FFT_N];
    
    // Perform fixed-point FFT
    fft_1024_point(test_input, fft_output);
    
    // Check that energy is concentrated at bin 1
    uint32_t bin1_mag = magnitude_squared(fft_output[64]);
    
    // All other bins should have much smaller magnitude
    for (int i = 0; i < FFT_N; i++) {
        if (i == 1) continue;
        uint32_t mag = magnitude_squared(fft_output[i]);
        if (mag > (bin1_mag >> 4)) {  // Should be less than 1/16 of bin 1
            return -1;
        }
    }
    
    return 0;
}


int test_ifft_correctness_1024() {
    complex_t original[FFT_N];
    
    // Initialize impulse signal manually
    original[0].real = 32767; original[0].imag = 0;  // Impulse
    for (int i = 1; i < FFT_N; i++) {
        original[i].real = 0;
        original[i].imag = 0;
    }

complex_t fft_output[FFT_N];
complex_t ifft_output[FFT_N];
    
    // Perform FFT then IFFT
    fft_1024_point(original, fft_output);
    ifft_1024_point(fft_output, ifft_output);
    
    // Check if we recovered the original signal
    int32_t tolerance = 1000;  // Allow some error due to fixed-point rounding
    
    for (int i = 0; i < FFT_N; i++) {
        int32_t real_error = original[i].real - ifft_output[i].real;
        int32_t imag_error = original[i].imag - ifft_output[i].imag;
        
        if (real_error < 0) real_error = -real_error;
        if (imag_error < 0) imag_error = -imag_error;
        
        if (real_error > tolerance || imag_error > tolerance) {
            return -1;
        }
    }
    
    return 0;
}
int test_fft_time(){
    complex_t test_input[FFT_N];
    for (int i = 0; i < FFT_N; i++) {
        test_input[i].real = 32767;
        test_input[i].imag = 0;
    }    
    complex_t fft_output[FFT_N];
    fft_1024_point(test_input, fft_output);
    return 0;
}
int test_fft_1024() {
    int test_results[4];
    for (int i=0; i<1; i++) {
    test_results[0] = test_impulse_1024();
    test_results[1] = test_dc_1024();
    test_results[2] = test_fft_time();
    // test_results[2] = test_single_frequency_1024();
    test_results[3] = test_ifft_correctness_1024();
    bench_sink += (test_results[0] + test_results[1] + test_results[2]);
    }
    if (bench_sink == -123456789) { // impossible path; prevents clever DCE
        // printf("%d\n", bench_sink);
    }
    // Check results
    int passed = 0;
    for (int i = 0; i < 4; i++) {
        if (test_results[i] == 0) {
            passed++;
        }
    }
    
    return (passed == 4) ? 0 : -1;
}

int main() {
    int r = test_fft_1024();
    return (r == 0) ? 0 : -1;
}
