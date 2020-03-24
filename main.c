#include "includes.h"

//#define USE_ALTERNATE_INPUT_DATA

/*
 * Test data used to feed the squaring logic that intermittently fails. This
 * is data from the original failure (dropbear SSH signing host key). The
 * failure isn't unique to this dataset - I'm just using it for consistency
 */
#if !defined(USE_ALTERNATE_INPUT_DATA)
static uint32_t     MpIntInputDigits[]={
/* 00-07 */ 0x0503525a, 0x05b845fa, 0x0c7fc034, 0x06c2f250, 0x0b4341ec, 0x00838383, 0x0298cbd8, 0x03ab06bb,
/* 08-15 */ 0x00f070ab, 0x0bd95a20, 0x00636daf, 0x015b85b0, 0x0594e753, 0x0cb19c6f, 0x0d4f02d9, 0x0cdd3be6,
/* 16-23 */ 0x07971428, 0x0be66f5f, 0x064435c4, 0x07b2ee5b, 0x0546d8e0, 0x0ca856ad, 0x0ac496a0, 0x0b429ee3,
/* 24-31 */ 0x0babb533, 0x022c737a, 0x0a559173, 0x029f4509, 0x0c9e367c, 0x03772e64, 0x00fb8625, 0x04029e2e, 
/* 32-39 */ 0x045ec768, 0x06bba7bf, 0x0511c436, 0x0e9cdca5, 0x05c79827, 0x0f4d865e, 0x02738dfe, 0x04114086,
/* 40-47 */ 0x0107a125, 0x0ecf94f7, 0x0c491d02, 0x0dcc228e, 0x0aa6104f, 0x0aa51002, 0x003bd55a, 0x0a7115d6,
/* 48-55 */ 0x02476d19, 0x09324c5b, 0x0e914e1c, 0x0612b244, 0x0cc85174, 0x0ac5b571, 0x0d31373f, 0x06aaca2e,
/* 56-63 */ 0x0bfb80a0, 0x042ea106, 0x09e177f3, 0x08546906, 0x0d29a073, 0x014798da, 0x0fc1ddd4, 0x0e0b2593,
/* 64-71 */ 0x0dc9f871, 0x03c172d0, 0x06e0e3b4, 0x080c060c, 0x03a93513, 0x02fcc6f5, 0x0a4e75f7, 0x03ae99b7,
/* 72-73 */ 0x04e339ea, 0x00000003
};
static mp_word ExpectedSqrResult[148]={
/* 000-003 */  0x0000000000000000, 0x001cac5dbd20ade4, 0x003ea84595163a48, 0x0069632f060686e8,
/* 004-007 */  0x005f2265c1b6f718, 0x0097822110fd66c6, 0x009cbac0fdc1face, 0x0073d0cbf665350a,
/* 008-011 */  0x003d9eeac3a0b28c, 0x0085f68955570e7e, 0x00878292cc870712, 0x00ce200943b258a2,
/* 012-015 */  0x00852e0894a443ef, 0x01029d1d104f044d, 0x00e72aa8e3aa4988, 0x0182b72c726b1bf8,
/* 016-019 */  0x01d7d6895cf23a0c, 0x0203c237854cc380, 0x01c9952a81fe7604, 0x01e589545705d7fe,
/* 020-023 */  0x01a7d21108edab4e, 0x0212f8bb55a5f0c2, 0x02290dc73a07bbc1, 0x0282e1f4009cea75,
/* 024-027 */  0x02885a0d9de82f21, 0x02a27a0f2c13c7ba, 0x02b98e21b0d255fd, 0x02bc0fc2e7c0554f,
/* 028-031 */  0x02f22b15b2dc4168, 0x02c74b877ab36f9c, 0x0350f40536f0d8ca, 0x02fe6de808e8728c,
/* 032-035 */  0x03143e3a120bf5a5, 0x030818ddbfc91830, 0x02b5c7efad8613a0, 0x040363fe04b31f00,
/* 036-039 */  0x03d7c18d83ca4a68, 0x04da5f101dcf4cd6, 0x0422cb762f2662ad, 0x0485c5c604ce7c34,
/* 040-043 */  0x03da6d7cfb6876f7, 0x048b666b8bea3d7a, 0x03ba866942cb1831, 0x04b73c84bb5fb572,
/* 044-047 */  0x04ad68a0e318f8a0, 0x05541076f7e7bae5, 0x04d4e8f5d4de17a6, 0x04ed87ecea5eb9c8,
/* 048-051 */  0x047f46a5de648b67, 0x0532bfa82d20a128, 0x05d0c91433a1a0b7, 0x059f339666c7534a,
/* 052-055 */  0x05c4bd6575851042, 0x0531733390ffa9d6, 0x06b46ff5bfc2ff87, 0x05d5d2f0177dabeb,
/* 056-059 */  0x0769d1434112772a, 0x06d4997e1a67ffba, 0x07a1ca7acb6b22ed, 0x0747cd00373d22f8,
/* 060-063 */  0x0702efc5ed6b5c0d, 0x06fc304de9fd70f1, 0x073e33aa35406f7c, 0x08082b5043cf0403,
/* 064-067 */  0x0843effd9cfb2155, 0x08d2a4490a3f7355, 0x08893f0415fbafc6, 0x08abb8214e5f71b5,
/* 068-071 */  0x0836a5d72881fd7e, 0x08a9362025ed9dcd, 0x07ace11e698e0cab, 0x08fc95d99f2a9c30,
/* 072-075 */  0x0875211b8cdb7340, 0x08dc69585cdea930, 0x079a81f9a48aec7f, 0x0847cef8fe841828,
/* 076-079 */  0x090df9e19e2e2cbf, 0x08cb712009d6b4f6, 0x0922254526c91e9b, 0x08af4ff3b6050c2f,
/* 080-083 */  0x0883f39cf13cfd1f, 0x07aea6abb35499b4, 0x0790270f01e471c4, 0x07501cd9068032f4,
/* 084-087 */  0x08542d1c7dfdbc8d, 0x081b1b697738ddfd, 0x081531edc3a2ab1d, 0x07ac1ea117cc1459,
/* 088-091 */  0x073d530b1a8e0b26, 0x06a63be9d981f2e7, 0x0672f86072bd47e0, 0x0724bfd3dc0e7788,
/* 092-095 */  0x0652116f3e381773, 0x069cc3b6886a9a53, 0x061884344c166e80, 0x06be8652ab887f59,
/* 096-099 */  0x05d7722d15618209, 0x06b0add682b7fabe, 0x0581c37737970611, 0x067c72b22832793b,
/* 100-103 */  0x051e090695387d7c, 0x060d07d83c4ddde4, 0x04ea55c56c6d9290, 0x05f7015c13e4006f,
/* 104-107 */  0x055cf711a07a6886, 0x064ebbf589030eb5, 0x0557926ad4f0b618, 0x060915ff2260b756,
/* 108-111 */  0x04ab94e004efb252, 0x051b33d5cdf638ec, 0x04605969c0c61b3c, 0x04a6c53e3df8fbb0,
/* 112-115 */  0x04b6faea3c69e8eb, 0x04c7fbec90fa0d64, 0x04d46e892e092f8b, 0x042e5abf3d06b61b,
/* 116-119 */  0x042ab3c435184674, 0x03e7887344131d69, 0x0398e3c913648f1c, 0x037edbbabd71f108,
/* 120-123 */  0x0354db61aae3c37f, 0x030c58ec8cb33ce1, 0x039630d18913707c, 0x02ddc2e0d8a21483,
/* 124-127 */  0x027a42efb428753f, 0x0286387a5420faa5, 0x0280d745124a44ca, 0x02235e2e2ba2c6c3,
/* 128-131 */  0x01a44c6351e12388, 0x01cddcb76dfb5e34, 0x01e3fd037e9e0025, 0x0151a53577883939,
/* 132-135 */  0x01621aa4416eb764, 0x013f4e73bbad7d03, 0x01333dc7a2f3bede, 0x00d020734dedb779,
/* 136-139 */  0x00b027e6736e1bdb, 0x00898e43431853e2, 0x0064faeb17cef1e1, 0x005399924dac190c,
/* 140-143 */  0x001ce4a1c89976a5, 0x00348cd3a30dfabc, 0x00325fbf62f727a5, 0x0011ff0e8602a22b,
/* 144-147 */  0x000000000b0bcd25, 0x000000000ea9adbe, 0x0000000000000000, 0x0000000000000000,
};
#else
static uint32_t     MpIntInputDigits[]={
/* 00-07 */ 0x80000000, 0x00000002, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
/* 09-15 */ 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
/* 16-23 */ 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
/* 24-31 */ 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
/* 32-39 */ 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
/* 40-47 */ 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
/* 48-55 */ 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
/* 56-63 */ 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
/* 64-71 */ 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
/* 72-73 */ 0x00000001, 0x00000001
};
static mp_word ExpectedSqrResult[148]; // we generated the expected result from the first invocation
#endif


/*
 * Do the squaring test until it fails or until we've done
 * an arbitrary-large number of attempts. Sometimes this
 * test app must be executed multiple times to fail, so
 * call it from a script that loops until this app
 * exits non-zero exit code
 */
int do_sqr_test(void) {

    int        iteration, indexMismatch;
    mp_int      a;

    set_cpu_affinity(0x01); /* run only on the first processor (keep test case simple) */
    for (iteration=0; iteration<500000 /* 500,000 */; iteration++) {

        // build mp_int structure
        a.used = countof(MpIntInputDigits);
        a.alloc = sizeof(a.dp); 
        a.sign  = MP_ZPOS;
        memcpy(a.dp, MpIntInputDigits, sizeof(MpIntInputDigits));

        // perform squaring operation
        fast_s_mp_sqr (&a, &a);

        if (iteration == 0) {
#if defined(USE_ALTERNATE_INPUT_DATA)
            // assume first invocation produced correct data, although
            // keep in mind it can fail on first attempt, so the second
            // iteration will actually have the correct data if that
            // happens
            memcpy(ExpectedSqrResult, SqrResult, sizeof(SqrResult));
#endif
            sleep(0); // helps to get faiure to occur - not certain why
        }

        // see if the failure occurred by 
        if (memcmp(SqrResult, ExpectedSqrResult, sizeof(SqrResult))) { 
            // issue hit. find word with wrong data
            for (indexMismatch=0; indexMismatch<countof(SqrResult); indexMismatch++) {
                if (SqrResult[indexMismatch] != ExpectedSqrResult[indexMismatch])
                    break;
            }
            // print results
            printf("Results Mismatch on Iteration %d - Index = %d, Byte Offset = 0x%04x\n",
                   iteration, indexMismatch,  indexMismatch*sizeof(mp_word));
            printf("Expected Word = 0x%08x:%08x, Actual Word = 0x%08x:%08x\n", 
                   (uint32_t)(ExpectedSqrResult[indexMismatch]>>32),
                   (uint32_t)(ExpectedSqrResult[indexMismatch]),
                   (uint32_t)(SqrResult[indexMismatch]>>32),
                   (uint32_t)(SqrResult[indexMismatch]));
            hex_dump_32bit("Expected Result", ExpectedSqrResult, sizeof(ExpectedSqrResult), indexMismatch*2);
            hex_dump_32bit("Actual Result", SqrResult, sizeof(SqrResult), indexMismatch*2);
            printf ("Press <enter> to exit\n"); fflush(stdin); getchar();
            return 1;
        }
    }
    return 0;
}     

int main(int argc, char ** argv) {
    return do_sqr_test(); 
}

