#include "includes.h"

//
// MpIntInputDigits is the input data into fast_s_mp_sqr()
// SqrResult is the output data, and ExpectedSqrResult is
// the expected results. After tuning fast_s_mp_sqr() to
// yield failures with even all-zero data, and after
// determning the input data pattern doesn't affect what
// corrupt data is yielded, I switched to using an all zero
// input / all-zero expected output, which makes it much
// easier to troubleshoot where the corrupt data is coming
// from
//
static mp_digit MpIntInputDigits[74];
static mp_digit SqrResult[148], ExpectedSqrResult[148];

#if defined(DYNAMIC_MP_DP)
void *mem_alloc_aligned(uint32_t bytes, uint32_t alignmentBytes, void **baseptr) {
    uint8_t *p = (uint8_t *)malloc(bytes+(alignmentBytes-1));
    *baseptr = p;
    return (void *) (((uintptr_t)p + (alignmentBytes-1)) & ~(alignmentBytes-1));
}
#endif

/*
 * Do the squaring test until it fails or until we've done
 * an arbitrary-large number of attempts. Sometimes this
 * test app must be executed multiple times to fail, so
 * call it from a script that loops until this app
 * exits non-zero exit code
 */
int do_sqr_test(void) {

    int        iteration, indexMismatch, ret;
    mp_int      a;

#if defined(DYNAMIC_MP_DP)
    //
    // optional logic to use an allocated buffer to hold the
    // input data rather than a static array built into the
    // mp_int structure. originally this was done to see if
    // corrupt data was coming from memory around the buffer,
    // but I've sinced determined that it's not. the logic is
    // still useful in debugging because having different
    // virtual addresses of the input buffer may affect where
    // we get corrupt data from (still determining). Also we
    // allocate the data with a 4K alignment just to make
    // the addresses cleaner
    //
    #define     BYTES_TO_ALLOCATE (256*1048576)
    void        *dpBase;
    a.dp = mem_alloc_aligned(BYTES_TO_ALLOCATE, 4096, &dpBase);
    memset(dpBase, 0xff, BYTES_TO_ALLOCATE);
    a.dp = (mp_digit *) (((uint8_t *)a.dp)+BYTES_TO_ALLOCATE/2);
#endif

    ret = 0;
    set_cpu_affinity(0x01); /* run only on the first processor (keep test case simple) */

    for (iteration=0; iteration<500000 /* 500,000 */; iteration++) {

        // build mp_int structure
        a.used = countof(MpIntInputDigits);
        a.alloc = countof(SqrResult); 
        a.sign  = MP_ZPOS;
        memcpy(a.dp, MpIntInputDigits, sizeof(MpIntInputDigits));

        // perform squaring operation
        fast_s_mp_sqr(&a, &a, SqrResult);

        // see if the failure occurred by
        if (memcmp(SqrResult, ExpectedSqrResult, sizeof(SqrResult))) { 

            // issue hit. find word with wrong data
            for (indexMismatch=0; indexMismatch<countof(SqrResult); indexMismatch++) {
                if (SqrResult[indexMismatch] != ExpectedSqrResult[indexMismatch])
                    break;
            }
            // print results
            printf("ipq8065-sqrbug: Mismatch on Iteration %d - Index = %d, Byte Offset = 0x%04x\n",
                   iteration, indexMismatch,  indexMismatch*sizeof(mp_digit));
            printf("Expected Word = 0x%08x, Actual Word = 0x%08x, &a.dp=0x%08x\n", 
                   (uint32_t)(ExpectedSqrResult[indexMismatch]),
                   (uint32_t)(SqrResult[indexMismatch]),
                   (uint32_t)(uintptr_t)a.dp);
            hex_dump_32bit("Expected Result", ExpectedSqrResult, sizeof(ExpectedSqrResult), indexMismatch);
            hex_dump_32bit("Actual Result", SqrResult, sizeof(SqrResult), indexMismatch);
            printf ("Press <enter> to continue\n"); fflush(stdin); getchar();
            ret = 1;
            break;
        }
    }

#if defined(DYNAMIC_MP_DP)
    free(dpBase);
#endif

    return ret;
}     

int main(int argc, char ** argv) {
    return do_sqr_test(); 
}

