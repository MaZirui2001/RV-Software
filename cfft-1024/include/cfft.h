/*
 * Header file for 16-Point Complex FFT Implementation (Fixed-Point Version)
 * 
 * This header defines the interface for the FFT functions and data structures.
 * Uses Q15 fixed-point arithmetic to avoid floating-point dependencies.
 */

#ifndef CFFT_H
#define CFFT_H

typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef short q15_t;
typedef int q31_t;
typedef long long q63_t;

// Fixed-point configuration
#define FIXED_POINT_BITS 15
#define FIXED_POINT_SCALE (1 << FIXED_POINT_BITS)  // 32768

// Fixed-point complex number structure
typedef struct {
    int32_t real;  // Q15 format: range [-1.0, 1.0) scaled by 32768
    int32_t imag;  // Q15 format: range [-1.0, 1.0) scaled by 32768
} complex_t;

// Function declarations
void fft_1024_point(const complex_t input[1024], complex_t output[1024]);
void ifft_1024_point(const complex_t input[1024], complex_t output[1024]);

// Test function
int test_fft_1024(void);

#endif // CFFT_H