#include "includes.h"

#define USE_ASSEMBLY_VERSION

/*
 * Test function to produce memory read corruption intermittently on my Netgear
 * X4S R7800. This code is from the libtommath library (fast_s_mp_sqr
 * function), which is used by the openwrt dropbear app that produce the
 * original symptom - failed SSH logins due to the wrong cyrpto signature of
 * the host key.
 *
 * This logic has been modified to elminate all elements not essential to
 * produce failure, including changing the sqauring option (multiply and adds)
 * to a simple bitwise OR, and to reduce the data size to a single 32-bit word
 * instead of 64-bit. This was done to make debugging the corruption easier,
 * including finding where the corrupt data came from.
 *
 * Because the issue is very sensitive to the specific layout of instructions
 * for the squaring loop I have pre-compiled the entire function. That way
 * future recompiles wont cause the logic to suddenly stop failing. You
 * can enable the C-language version below by commenting out the
 * #define USE_ASSEMBLY_VERSION at the top of this file.
 * 
 * The issue on my router is likely a faulty IPQ8065 or memory issue, but I
 * wrote this app to have other tests theirs to be sure. Note that the issue is
 * very sensitive to timing
 */ 

#if defined(USE_ASSEMBLY_VERSION)
#if !defined(DYNAMIC_MP_DP)
    /* version when mp_int.dp is an array instantiated in the structure */ 
void __attribute__ ((noinline)) fast_s_mp_sqr (mp_int * a, mp_int * b, mp_digit SqrResult[]) {
    asm(".word 0xe92d43f0");    /* push     {r4, r5, r6, r7, r8, r9, lr} */
    asm(".word 0xe1a04000"); 	/* mov	r4, r0                           */ 
    asm(".word 0xe494500c"); 	/* ldr	r5, [r4], #12                    */
    asm(".word 0xe3a0c000"); 	/* mov	ip, #0                           */
    asm(".word 0xe1a05085"); 	/* lsl	r5, r5, #1                       */
    asm(".word 0xe15c0005"); 	/* cmp	ip, r5                           */
    asm(".word 0xa8bd83f0"); 	/* popge	{r4, r5, r6, r7, r8, r9, pc} */  
    asm(".word 0xe590e000"); 	/* ldr	lr, [r0]                         */
    asm(".word 0xe24e1001"); 	/* sub	r1, lr, #1                       */
    asm(".word 0xe151000c"); 	/* cmp	r1, ip                           */
    asm(".word 0xa1a0100c"); 	/* movge	r1, ip                       */
    asm(".word 0xe2813001"); 	/* add	r3, r1, #1                       */
    asm(".word 0xe04c6001"); 	/* sub	r6, ip, r1                       */
    asm(".word 0xe0847101"); 	/* add	r7, r4, r1, lsl #2               */
    asm(".word 0xe041100c"); 	/* sub	r1, r1, ip                       */
    asm(".word 0xe08ee001"); 	/* add	lr, lr, r1                       */
    asm(".word 0xe15e0003"); 	/* cmp	lr, r3                           */
    asm(".word 0xe0846106"); 	/* add	r6, r4, r6, lsl #2               */
    asm(".word 0xa1a0e003"); 	/* movge	lr, r3                       */
    asm(".word 0xe0833001"); 	/* dd	r3, r3, r1                       */
    asm(".word 0xe1a030c3"); 	/* asr	r3, r3, #1                       */
    asm(".word 0xe153000e"); 	/* cmp	r3, lr                           */
    asm(".word 0xa1a0300e"); 	/* movge	r3, lr                       */ 
    asm(".word 0xe320f000"); 	/* nop	{0}                              */
    asm(".word 0xe3a01000"); 	/* mov	r1, #0                           */
    asm(".word 0xe1a0e001"); 	/* mov	lr, r1                           */
    asm(".word 0xe15e0003"); 	/* cmp	lr, r3                           */
    asm(".word 0xa782110c"); 	/* strge	r1, [r2, ip, lsl #2]         */
    asm(".word 0xa28cc001"); 	/* addge	ip, ip, #1                   */
    asm(".word 0xaaffffe6"); 	/* bge	8fc <fast_s_mp_sqr+0x14>         */
    asm(".word 0xe796810e"); 	/* ldr	r8, [r6, lr, lsl #2]             */
    asm(".word 0xe28ee001"); 	/* add	lr, lr, #1                       */
    asm(".word 0xe4179004"); 	/* ldr	r9, [r7], #-4                    */
    asm(".word 0xe1888009"); 	/* orr	r8, r8, r9                       */
    asm(".word 0xe1811008"); 	/* orr	r1, r1, r8                       */
    asm(".word 0xeafffff5"); 	/* b	950 <fast_s_mp_sqr+0x68>         */
}
#else
    /*  version when mp_int.dp is an array instantiated in the structure */ 
void __attribute__ ((noinline)) fast_s_mp_sqr (mp_int * a, mp_int * b, mp_digit SqrResult[]) {
    asm(".word 0xe92d41f0");  /* push	{r4, r5, r6, r7, r8, lr}     */
    asm(".word 0xe3a0c000");  /* mov	ip, #0                       */
    asm(".word 0xe5905000");  /* ldr	r5, [r0]                     */
    asm(".word 0xe1a05085");  /* lsl	r5, r5, #1                   */
    asm(".word 0xe15c0005");  /* cmp	ip, r5                       */
    asm(".word 0xa8bd81f0");  /* popge	{r4, r5, r6, r7, r8, pc}     */
    asm(".word 0xe590e000");  /* ldr	lr, [r0]                     */
    asm(".word 0xe590400c");  /* ldr	r4, [r0, #12]                */
    asm(".word 0xe24e1001");  /* sub	r1, lr, #1                   */
    asm(".word 0xe151000c");  /* cmp	r1, ip                       */
    asm(".word 0xa1a0100c");  /* movge	r1, ip                       */
    asm(".word 0xe04c6001");  /* sub	r6, ip, r1                   */
    asm(".word 0xe2813001");  /* add	r3, r1, #1                   */
    asm(".word 0xe0846106");  /* add	r6, r4, r6, lsl #2           */
    asm(".word 0xe0844101");  /* add	r4, r4, r1, lsl #2           */
    asm(".word 0xe041100c");  /* sub	r1, r1, ip                   */
    asm(".word 0xe08ee001");  /* add	lr, lr, r1                   */
    asm(".word 0xe15e0003");  /* cmp	lr, r3                       */
    asm(".word 0xa1a0e003");  /* movge	lr, r3                       */
    asm(".word 0xe0833001");  /* add	r3, r3, r1                   */
    asm(".word 0xe1a030c3");  /* asr	r3, r3, #1                   */
    asm(".word 0xe153000e");  /* cmp	r3, lr                       */
    asm(".word 0xa1a0300e");  /* movge	r3, lr                       */
    asm(".word 0xe320f000");  /* nop	{0}                          */
    asm(".word 0xe3a01000");  /* mov	r1, #0                       */
    asm(".word 0xe1a0e001");  /* mov	lr, r1                       */
    asm(".word 0xe15e0003");  /* cmp	lr, r3                       */
    asm(".word 0xa782110c");  /* strge	r1, [r2, ip, lsl #2]         */
    asm(".word 0xa28cc001");  /* addge	ip, ip, #1                   */
    asm(".word 0xaaffffe5");  /* bge	9e0 <fast_s_mp_sqr+0x10>     */
    asm(".word 0xe796710e");  /* ldr	r7, [r6, lr, lsl #2]         */
    asm(".word 0xe28ee001");  /* add	lr, lr, #1                   */
    asm(".word 0xe4148004");  /* ldr	r8, [r4], #-4                */
    asm(".word 0xe1877008");  /* orr	r7, r7, r8                   */
    asm(".word 0xe1811007");  /* orr	r1, r1, r7                   */
    asm(".word 0xeafffff5");  /* b	a38 <fast_s_mp_sqr+0x68>         */
}
#endif /* #else of #if defined(USE_ASSEMBLY_VERSION) */

#else /* #if defined(USE_ASSEMBLY_VERSION) */
void __attribute__ ((noinline)) fast_s_mp_sqr (mp_int * a, mp_int * b, mp_digit SqrResult[]) {

    int                 pa;
    int                 ix;
    int                 iz;
    mp_digit            *tmpx;

    pa = a->used + a->used;

    for (ix = 0; ix < pa; ix++) { 

        int      tx, ty, iy;
        mp_digit _W;
        mp_digit *tmpy;

        /* clear counter */
        _W = 0;

        /* get offsets into the two bignums */
        ty = MIN(a->used-1, ix);
        tx = ix - ty;

        /* setup temp aliases */
        tmpx = a->dp + tx;
        tmpy = a->dp + ty;

        /* this is the number of times the loop will iterrate, essentially
          while (tx++ < a->used && ty-- >= 0) { ... }
         */
        iy = MIN(a->used-tx, ty+1);

        /* now for squaring tx can never equal ty 
         * we halve the distance since they approach at a rate of 2x
         * and we have to round because odd cases need to be executed
         */
        iy = MIN(iy, ((ty-tx)+1)>>1);

        /*
         * placeholder "nop", to experiment with various memory barrier instructions
         */
        asm("nop");

        /*
         * this is the "squaring loop" that intermittently
         * yields the wrong result
         */
        for (iz = 0; iz < iy; iz++) {
            _W |= (*tmpx++)|(*tmpy--);
        }
        // save result, which is used by caller to check if failure occurred
        SqrResult[ix] = _W; 
    }
}
#endif /* #else of #if defined(USE_ASSEMBLY_VERSION) */

