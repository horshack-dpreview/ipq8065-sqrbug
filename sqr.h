#ifndef _INCL_SQR
#define _INCL_SQR

#define MP_ZPOS       0   /* positive integer */
#define MP_NEG        1   /* negative */

#define MP_WARRAY   (0x200)
#define DIGIT_BIT   (28)
#define MP_MASK     ((((mp_digit)1)<<((mp_digit)DIGIT_BIT))-((mp_digit)1)) /* result: 0x0FFFFFFF */

//#define DYNAMIC_MP_DP

typedef uint32_t mp_digit;
typedef uint64_t mp_word;

typedef struct  {
    int used, alloc, sign;
#if !defined(DYNAMIC_MP_DP)
    mp_digit dp[148];
#else
    mp_digit *dp;
#endif
} mp_int;

extern void fast_s_mp_sqr (mp_int * a, mp_int * b, mp_digit SqrResult[]);

#endif /* #ifndef _INCL_SQR */
