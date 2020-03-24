#include "includes.h"

volatile mp_word SqrResult[148];

void __attribute__ ((noinline)) __attribute__((optimize("O0"))) dummy_func() {
}

/*
 * Test function to square an array of numbers that fails
 * intermittently on my Netgear X4S R7800. This code is from the
 * libtommath library, which is used by the openwrt dropbear app which
 * produce the original symptom - failed SSH logins due to the wrong
 * cyrpto signature of the host key.
 *
 * This logic has been modified to elminate all elements not essential
 * to produce failure, including some aspects of the original squaring
 * logic, so it's not performing same operations as the original
 * version. Also, instead of the final squared data output we use the
 * intermediate data for validation of the result, which is simpler to
 * evaluate and is just after the squaring loop that yields the wrong
 * data.
 *
 * To match the original failing function's behavior it's necessary to
 * specify certain local variables as register vars and also to invoke
 * a dummy func - these are both used to control GCC's register
 * allocations to get the assembly output in a form that produces the
 * issue.
 *
 * The issue on my router is likely a faulty IPQ8065 or memory issue,
 * but I wrote this app to have other tests theirs to be sure.
 *
 * The issue is very sensitive to timing and somewhat sensitive to
 * data patterns, which implies a memory issue, either on the data or
 * address. For example this function can be made to fail must less
 * often by replacing the  placeholder "nop" instruction with an ARM
 * data-barrier instruction such as "dsb". The purpose of the
 * placeholder "nop" is to limit the difference between the failing
 * and non-failing versions of the function to a single instruction to
 * limit code movement
 */ 
void fast_s_mp_sqr (mp_int * a, mp_int * b) {

    int                 olduse, res, pa;
    int                 ix;
    int                 iz;
    mp_digit            W[MP_WARRAY], *tmpx;
    register mp_word    W1 asm("d0");

    pa = a->used + a->used;
    dummy_func();

    W1 = 0;
    for (ix = 0; ix < pa; ix++) { 

        int      tx, ty, iy;
        register mp_word _W asm("d1");
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
         * placeholder "nop". if it's replaced with a dsb instruction
         * the failure never occurs
         */
        asm("nop");
        //asm("dsb");
        //asm("dmb");

        /*
         * this is the "squaring loop" that intermittently
         * yields the wrong result
         */
        for (iz = 0; iz < iy; iz++) {
            _W += ((mp_word)*tmpx++)*((mp_word)*tmpy--);
        }
        // save intermediate result. used later by caller to check if failure occurred
        SqrResult[ix] = _W; 

        /* double the inner product and add carry */
        _W = _W + _W + W1;

        W[ix] = (mp_digit)(_W & MP_MASK); // original funcion storing of result
    }
}
