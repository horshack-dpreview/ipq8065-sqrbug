# ipq8065-sqrbug
I have a Neatgea R7800 Nighthawk X4S AC2 router that reported SSH authentication errors for about 30% of my SSH login attempts. Dropbear on my OpenWrt installation was reporting a key error during the diffie-hellman key exchange. I narrowed the issue down to a single routine in Dropbear and was able to reproduce the problem in a standalone-app I created that extracted the logic for that routine. I further simplified the logic down to the bare essential necessary to reproduce the error. The source and binary in this repository represents that effort.

Here are the emails I posted to the Dropbear mailing list:
```
From: Horshack ‪‬
Sent: Wednesday, March 18, 2020 9:36 AM
To: dropbear@ucc.asn.au <dropbear@ucc.asn.au>
Subject: SSH key exchange fails 30-70% of the time on Netgear X4S R7800

Hi,

I have a strange issue on my Netgear X4S R7800. Running either DD-WRT or 
OpenWrt, approximately 30-70% of my SSH login attempts fail. For OpenSSH 
clients the error reported is "error in libcrypto". For the PuTTY client the 
error is more descriptive - "Signature from server's host key is invalid". The 
failure occurs even when using the OpenSSH client built in to OpenWrt itself 
(ie, SSH'ing into the router from the router via an existing remote SSH 
session).

The failure appears to be at the tail end of the key exchange, before 
authentication. I've tried varying the cipher (aes128-ctr / aes256-ctr), the 
MAC (hmac-sha1 / hmac-sha2-256), and the key exchange algo (curve25519-sha256 / 
curve25519-sha...@libssh.org / diffie-hellman-group14-sha256 / 
diffie-hellman-group14-sha1) but the intermittent failure still occurs. The 
frequency of failure is about the same for all these configuration options 
except for diffie-hellman-group14-sha256, which fails much more frequently - it 
sometimes takes hundreds of attempts to succeed. Perhaps that will provide a 
clue to the underlying cause.

Once an SSH login succeeds the connection is stable. However if I initiate a 
manual rekey operation via ~R then the key re-exchange fails. The router is 
otherwise very stable with no noticeable issues.

I'm an embedded firmware engineer but have never worked on DD-WRT/OpenWrt 
firmware or dropbear. I have a conceptual understanding of the key exchange 
algo but haven't looked at the actual code of any implementation including 
Dropbear's. I'm seek ideas on how to troubleshoot this issue. Considering the 
problem is intermittent I'm thinking it's some variant in the key 
generation/exchange algorithm that's failing due to some issue with the router, 
or a more remote possibility, an issue with the Dropbear implementation.

Here are pastebin links to the PuTTY full debug logs (w/raw data dumps) for 
both the failure and success cases:
Failure Case: https://pastebin.com/MS2BtFmW
Success Case: https://pastebin.com/c4j66Ga9

The only message I see from dropbear for a failed connection attempt is:

authpriv.info dropbear[15948]: Child connection from 192.168.1.249:54819
authpriv.info dropbear[15948]: Exit before auth: Disconnect received

Thanks!
```

```
Horshack ‪‬ Thu, 19 Mar 2020 00:44:14 -0700
Update - I cloned and built the dbclient source so I could enable the debug 
tracing facility to get more information about the 'Bad hostkey signature'. The 
intermittent failure is detected in recv_msg_kexdh_reply() -> buf_rsa_verify() 
-> mp_cmd(). If I bypass the buf_rsa_verify() call then the session proceeds 
normally without issue, which indicates everything else in the key exchange is 
working 100% of the time. I'll dig deeper to see why the signed host key sent 
by the server is wrong.
```

```
Horshack ‪‬ Fri, 20 Mar 2020 00:30:12 -0700

Update - I have isolated the intermittent issue down to the interchangeable 
functions s_mp_exptmod_fast() and s_mp_exptmod() - by default 
s_mp_exptmod_fast() is compiled instead of s_mp_exptmod() 
[BN_MP_EXPTMOD_FAST_C] but both functions intermittently fail and I decided to 
use s_mp_exptmod() as my focus because it's slightly simpler.

s_mp_exptmod() is called indirectly by rsa.c::buf_put_rsa_sign()'s call to 
mp_exptmod(). For the intermittent failing case if I call mp_exptmod() / 
s_mp_exptmod() immediately again with the same source mp_int structures it 
yields the correct data. Example - debug code bolded:

    DEF_MP_INT(rsa_s_backup);
    DEF_MP_INT(rsa_s_backup_2);

    mp_copy (&rsa_s, &rsa_s_backup);
    mp_copy (&rsa_s, &rsa_s_backup_2);

    if (mp_exptmod(&rsa_tmp1, key->d, key->n, &rsa_s) != MP_OKAY) {
        dropbear_exit("RSA error");
    }
    if (mp_exptmod(&rsa_tmp1, key->d, key->n, &rsa_s_backup) != MP_OKAY) {
        dropbear_exit("RSA error");
    }
    if (mp_exptmod(&rsa_tmp1, key->d, key->n, &rsa_s_backup_2) != MP_OKAY) {
        dropbear_exit("RSA error");
    }
    printf("after mp_exptmod\n");
    dump_mp_int("rsa_s", &rsa_s);
    dump_mp_int("rsa_s_backup", &rsa_s_backup);
    dump_mp_int("rsa_s_backup_2", &rsa_s_backup_2);
    comp_mp_int("rsa_s", "rsa_s_backup", &rsa_s, &rsa_s_backup);
    comp_mp_int("rsa_s_backup", "rsa_s_backup_2", &rsa_s_backup, 
&rsa_s_backup_2);
    mp_clear(&rsa_s_backup);
    mp_clear(&rsa_s_backup_2);

Sample output from a failure, which contains the first portion of each 
mp_int->dp. Bolded text has wrong data:

after mp_exptmod
rsa_s [0xbef6c358]:
  0000  4a 00 00 00 c0 00 00 00 00 00 00 00 30 e1 8f 00  J...........0...
rsa_s->dp [0x008fe130]:
  0000  05 fb c0 0f 68 91 ff 0a 9f 05 57 0b 35 a2 bd 05  ....h.....W.5...
  0010  57 ec a0 0b 34 3c b1 0f fa 8b b5 08 ed aa 9c 04  W...4<..........
  0020  7e 88 bb 04 12 42 51 05 9a 6d 7d 0a 98 ef 12 0c  ~....BQ..m}.....
  0030  76 e0 f4 0f ea 89 d7 0c 87 b0 76 03 12 a1 2d 0e  v.........v...-.
  0040  d7 3c df 06 0f 54 92 04 23 90                    .<...T..#.
rsa_s_backup [0xbef6c398]:
  0000  4a 00 00 00 c0 00 00 00 00 00 00 00 00 d8 8f 00  J...............
rsa_s_backup->dp [0x008fd800]:
  0000  ec 9f a0 01 d4 8e e8 07 c3 ae df 0b 45 61 e6 06  ............Ea..
  0010  a1 99 59 03 d7 49 24 02 50 a6 ac 0a de a2 5c 0d  ..Y..I$.P.....\.
  0020  cb b7 3c 05 33 cb da 08 28 10 f2 04 14 69 d6 07  ..<.3...(....i..
  0030  8c 8e a5 04 f5 fc 92 0c ba 88 d9 04 71 b4 b2 08  ............q...
  0040  bc 4f c7 0d de 73 f9 06 0d bf                    .O...s....
rsa_s_backup_2 [0xbef6c3a8]:
  0000  4a 00 00 00 c0 00 00 00 00 00 00 00 e0 d1 8f 00  J...............
rsa_s_backup_2->dp [0x008fd1e0]:
  0000  ec 9f a0 01 d4 8e e8 07 c3 ae df 0b 45 61 e6 06  ............Ea..
  0010  a1 99 59 03 d7 49 24 02 50 a6 ac 0a de a2 5c 0d  ..Y..I$.P.....\.
  0020  cb b7 3c 05 33 cb da 08 28 10 f2 04 14 69 d6 07  ..<.3...(....i..
  0030  8c 8e a5 04 f5 fc 92 0c ba 88 d9 04 71 b4 b2 08  ............q...
  0040  bc 4f c7 0d de 73 f9 06 0d bf                    .O...s....
rsa_s and rsa_s_backup differ

Sometimes it's the second or third call that yields the incorrect data. In this 
instance it was the second call:
after mp_exptmod
rsa_s [0xbe9a6358]:
  0000  4a 00 00 00 c0 00 00 00 00 00 00 00 30 c1 40 02  J...........0.@.
rsa_s->dp [0x0240c130]:
  0000  25 b9 db 00 ec 62 00 0d 80 2d b0 0d 00 13 d3 06  %....b...-......
  0010  3f ec 8b 0a af 5d e9 03 2d f4 4b 0c 6c 3c 72 08  ?....]..-.K.l<r.
  0020  5d 52 6a 08 21 4c dd 01 a2 59 1a 03 33 16 97 0f  ]Rj.!L...Y..3...
  0030  c7 69 c2 08 0b 61 d6 03 b9 86 fc 01 27 15 c8 0c  .i...a......'...
  0040  dd 03 b1 04 78 c7 9f 0f d8 9c                    ....x.....
rsa_s_backup [0xbe9a6398]:
  0000  4a 00 00 00 c0 00 00 00 00 00 00 00 00 b8 40 02  J.............@.
rsa_s_backup->dp [0x0240b800]:
  0000  df 86 0c 0a 6c 2f 68 09 f9 a1 37 01 26 02 e7 0b  ....l/h...7.&...
  0010  69 5c b8 0e 0b 95 3a 0d 26 24 00 0e 97 6f dc 0b  i\....:.&$...o..
  0020  64 95 ed 0a c0 75 53 03 66 3d ff 0b 26 4b ce 09  d....uS.f=..&K..
  0030  89 12 d2 03 9b 9b 0b 09 19 2c 5a 00 2c 99 fc 0b  .........,Z.,...
  0040  ea ad 61 09 38 e1 6a 0a 49 a5                    ..a.8.j.I.
rsa_s_backup_2 [0xbe9a63a8]:
  0000  4a 00 00 00 c0 00 00 00 00 00 00 00 e0 b1 40 02  J.............@.
rsa_s_backup_2->dp [0x0240b1e0]:
  0000  25 b9 db 00 ec 62 00 0d 80 2d b0 0d 00 13 d3 06  %....b...-......
  0010  3f ec 8b 0a af 5d e9 03 2d f4 4b 0c 6c 3c 72 08  ?....]..-.K.l<r.
  0020  5d 52 6a 08 21 4c dd 01 a2 59 1a 03 33 16 97 0f  ]Rj.!L...Y..3...
  0030  c7 69 c2 08 0b 61 d6 03 b9 86 fc 01 27 15 c8 0c  .i...a......'...
  0040  dd 03 b1 04 78 c7 9f 0f d8 9c                    ....x.....
rsa_s and rsa_s_backup differ

I have heavily instrumented s_mp_exptmod() but due to the complexity of the 
calcualtions performed it's proving very difficult to root down to the issue. 
What I can tell so far is the failure point within s_mp_exptmod() varies from 
instance to instance, which is odd because the only potential variant between 
my three, back-to-back invocations are the memory allocations (buffer 
locations) triggered by mp_exptmod(), although the invocations usually get 
provided the same buffer addresses. I tried various scaffolding code on the 
core memory allocation routines to isolate any buffer overruns/overwrites the 
logic might be performing, including padding each allocation by a large block 
of bytes, but the intermittent failure case still occurs. The behavior I'm 
observing almost appears as if the execution context is being corrupted (ie, 
processor registers) because the failure point moves around the various 
elements of the logic within the routine from one failure to the next - 
sometimes I see an early-stage mp_int structure with the wrong data, sometimes 
one that has undergone many transformations - all within s_mp_exptmod().

Do you know if OpenWRT has any way to disable SMP at runtime, or a method or 
technique to provide a critical section around a block of code to prevent any 
preemptive task switches?
```

```
Horshack ‪‬ Tue, 24 Mar 2020 08:08:12 -0700

I have one of the failure paths isolated down to a single corrupt 64-bit word 
in memory, which required a significant amount of code instrumentation to 
achieve. I implemented a code execution history buffer that gets filled at 
various checkpoints within s_mp_exptmod() and some of the modules called by it. 
To facilitate this history mechanism I packaged all of s_mp_exptmod()'s local 
variables inside a structure , which consists of saving the local scalar vars 
in addition to crc32's of all the mp_int data structures with a separate crc32 
of the mp_int.dp payload (data). When a failure occurs, ie one or more of the 
three back-to-back debug invocations of s_mp_exptmod yields a mismatching 
signed key result, I  dump out the history elements for each of the invocations 
to determine the first code checkpoint where failing invocation departed from 
the known correct invocation.

Here's a sample capture demonstrating.

Format is event #:, source code line #, crc32 of local scalars, crc32 of mp_int 
structures (minus dp field), and crc32 of all the mp_int dp data payloads. In 
this sample, the crc32 of the dp data payload is different, which causes all 
subsequent crc32's for the remainder of the invocation to be difference since 
the data propagates through all the subsequent calculations performed in the 
routine.

1554: line=0492, crcLocalVars=6a08573e, crcMpIntNoDp=ab967993, 
crcMpIntDp=ded4078e crcRes=2554be5b, 0021 0005 0016 0002 0061 0003 0001
1555: line=0488, crcLocalVars=7dc8fe2c, crcMpIntNoDp=ad3e197a, 
crcMpIntDp=e71d5c11 crcRes=5ef59250, 0021 0005 0016 0002 0061 0004 0001
1556: line=2049, crcLocalVars=7dc8fe2c, crcMpIntNoDp=ad3e197a, 
crcMpIntDp=e71d5c11 crcRes=5ef59250, 0021 0005 0016 0002 0061 0004 0001
1557: line=2062, crcLocalVars=7dc8fe2c, crcMpIntNoDp=ab967993, 
crcMpIntDp=21b13223 crcRes=a43fde70, 0021 0005 0016 0002 0061 0004 0001
1558: line=0492, crcLocalVars=7dc8fe2c, crcMpIntNoDp=ab967993, 
crcMpIntDp=21b13223 crcRes=a43fde70, 0021 0005 0016 0002 0061 0004 0001
1559: line=0501, crcLocalVars=7a3e1d2a, crcMpIntNoDp=ad3e197a, 
crcMpIntDp=7691624d crcRes=6d1388bc, 0021 0005 0016 0002 0061 0005 0001

1554: line=0492, crcLocalVars=6a08573e, crcMpIntNoDp=ab967993, 
crcMpIntDp=ded4078e crcRes=2554be5b, 0021 0005 0016 0002 0061 0003 0001
1555: line=0488, crcLocalVars=7dc8fe2c, crcMpIntNoDp=ad3e197a, 
crcMpIntDp=e71d5c11 crcRes=5ef59250, 0021 0005 0016 0002 0061 0004 0001
1556: line=2049, crcLocalVars=7dc8fe2c, crcMpIntNoDp=ad3e197a, 
crcMpIntDp=e71d5c11 crcRes=5ef59250, 0021 0005 0016 0002 0061 0004 0001
1557: line=2062, crcLocalVars=7dc8fe2c, crcMpIntNoDp=ab967993, 
crcMpIntDp=a2639ce4 crcRes=74f7dec6, 0021 0005 0016 0002 0061 0004 0001
1558: line=0492, crcLocalVars=7dc8fe2c, crcMpIntNoDp=ab967993, 
crcMpIntDp=a2639ce4 crcRes=74f7dec6, 0021 0005 0016 0002 0061 0004 0001
1559: line=0501, crcLocalVars=7a3e1d2a, crcMpIntNoDp=ad3e197a, 
crcMpIntDp=5e3343d2 crcRes=517ed1b0, 0021 0005 0016 0002 0061 0005 0001

I initially found the failure occurs at seemingly random places, affected 
mostly by the variances of code/data placement between builds, which also 
affects the frequency of failure. Through a lot of trial and error I was able 
to tease the failure down to one of the simplest code paths (fast_s_mp_sqr), 
which required balancing debug code placement to keep the movement of the 
failure in control. fast_s_mp_sqr() does only basic arithmetic and is easy to 
follow. I haven't yet determined if the corrupt data is pre-calculation or 
post-calculation due to the limits of how much data I can snapshot in the 
history buffer. Nevertheless I expanded the history mechanism to snapshot the 
specific mp_int that usually is corrupted via this path (s_mp_exptmod's local 
res structure).

Here is correct vs corrupt mp_init at the specific execution point where it 
departs from the previous correction invocation. The data fields prefixed by : 
are the actual content of the mp_int - I've highlighted the mismatching crc32's 
and the mismatching 64-bit word:

Correct invocation:
8057: line=2062, crcLocalVars=1d8f10b6, crcMpIntNoDp=80a0f0a7, 
crcMpIntDp=e92a3e1f crcRes=02003870, 0018 0005 0020 0002 0016 0002 0000
: 05297100 04f1e4e6 0fb47d28 0ab5d584 00b2778c 08656465 02cc79bb 05e280c3 - 
073117bc 037170a2 0603ef41 0a73c7af 0388c6cd 08b543fa 055d90c9 006afe46
: 0c4d0d2b 0b8753bf 0ba6b917 0dbc26af 0d5d541f 03cbd888 0a8b07bb 06ce141b - 
0f2e2cdc 0d83829c 00b9e992 007a007e 0b35c3fa 0f97fa98 078b16e2 05681c5a
: 09e81cad 0fcb1b35 0f017b34 0828f9c8 08253004 02f4139f 07b97efe 03a2c2c6 - 
0baf31f0 038dc84d 0ec2028d 0a4d2163 0b3d8f14 03a5b8a1 07656722 0636f515
: 047c6a4e 0249e773 074fdaae 0c7affcb 025e144e 0e6e524b 0369a7e6 005e5b18 - 
07359ab7 094aa102 06e091dc 048578b3 0f2023d6 09e16318 0fb25f70 091e7d0c
: 00e038fe 01fe0be1 0c879fba 055feb36 05135c48 063ef5c4 062acf74 0e2ee213 - 
0b32d4b4 01ac1beb 0df27135 0645d3a2 02f54fab 04524d06 0e21e0a0 01a58051
: 0d0dd311 0b10815a 08044871 0bec8042 0473b083 0d99e620 0db94b72 07398f84 - 
06930d29 021f81cd 0e96625a 0ffa3c78 0c9908d6 0fd6f904 0f5dcfd9 0bd6e140
: 0357bd4b 0488f3a9 00ed811d 0c8a129f 0bde5ab5 0c61d340 042eea72 01fe06f5 - 
018c9e3d 025ede93 0ce5786c 00c174de 0479c67d 06c711f5 052ebca1 093bf956
: 042b9b5e 06a62fce 0eef5130 0065890a 0ed4ef4d 0adc823d 0b7ab96f 04639d68 - 
0484c7b5 0135f153 0818067f 00cffc19 0097dcba 016e355b 002e3d3e 051065cb
: 0b41750c 049fb50f 0be87386 0d76e872 0de83a61 04f8c371 07daa886 03a70e50 - 
0c79ea89 016660c2 0963ebd6 09d9b469 0abd18ff 02c370ac 0ad5b8ba 04846255
: 0e7c9e10 03662210 00000011 00000000 00000000 00000000 00000000 00000000 - 
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000

Corrupt invocation:
8057: line=2062, crcLocalVars=1d8f10b6, crcMpIntNoDp=80a0f0a7, 
crcMpIntDp=5a521526 crcRes=86bd8450, 0018 0005 0020 0002 0016 0002 0000
: 05297100 04f1e4e6 0fb47d28 0ab5d584 00b2778c 08656465 02cc79bb 05e280c3 - 
073117bc 037170a2 0603ef41 0a73c7af 0388c6cd 08b543fa 055d90c9 006afe46
: 0c4d0d2b 0b8753bf 0ba6b917 0dbc26af 0d5d541f 03cbd888 0a8b07bb 06ce141b - 
0f2e2cdc 0d83829c 00b9e992 007a007e 0b35c3fa 0f97fa98 078b16e2 05681c5a
: 09e81cad 0fcb1b35 0f017b34 0828f9c8 08253004 02f4139f 07b97efe 03a2c2c6 - 
0baf31f0 038dc84d 0ec2028d 0a4d2163 0b3d8f14 03a5b8a1 07656722 0636f515
: 047c6a4e 0249e773 074fdaae 0c7affcb 025e144e 0e6e524b 0369a7e6 005e5b18 - 
07359ab7 094aa102 06e091dc 048578b3 0f2023d6 09e16318 0fb25f70 091e7d0c
: 00e038fe 01fe0be1 0c879fba 055feb36 05135c48 063ef5c4 062acf74 0e2ee213 - 
0b32d4b4 01ac1beb 0df27135 0645d3a2 02f54fab 04524d06 0e21e0a0 01a58051
: 0d0dd311 0b10815a 08044871 0bec8042 0473b083 0d99e620 0db94b72 07398f84 - 
06930d29 021f81cd 0e96625a 0ffa3c78 0c9908d6 0fd6f904 0f5dcfd9 0bd6e140
: 0357bd4b 0488f3a9 00ed811d 0c8a129f 0bde5ab5 0c61d340 042eea72 01fe06f5 - 
018c9e3d 025ede93 0ce5786c 00c174de 0479c67d 06c711f5 052ebca1 093bf956
: 042b9b5e 06a62fce 0eef5130 0065890a 0ed4ef4d 0adc823d 0b7ab96f 04639d68 - 
0484c7b5 0135f153 0818067f 00cffc19 0097dcba 016e355b 002e3d3e 051065cb
: 0b41750c 049fb50f 0be87386 0d76e872 0de83a61 07156229 072adcf7 03a70e50 - 
0c79ea89 016660c2 0963ebd6 09d9b469 0abd18ff 02c370ac 0ad5b8ba 04846255
: 0e7c9e10 03662210 00000011 00000000 00000000 00000000 00000000 00000000 - 
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000

I don't see any immediate relationship between the corrupt vs expected data or 
any unique attributes of the corrupt data over multiple captures I've done. The 
above mp_int is post-execution of fast_s_mp_sqr(), so any corruption occurring 
within its execution will get folded in and propagated into a form that wont be 
immediately recognizable since it's undergone arithmetic operations within the 
routine.

The fact the corruption is always a single 64-bit word is a good clue. 
fast_s_mp_sqr() uses 64-bit scalars (mp_word) in its carry arithmetic logic   - 
I'll be looking into the disassembly of the routine to dig deeper.

For reference here is the history structures used for the above dumps:

typedef struct _LOCAL_VARS { // local vars of s_mp_exptmod() packaged into a 
struct
    mp_int *G;
    mp_int *X;
    mp_int *P;
    mp_int *Y;
    mp_int M[TAB_SIZE];
    mp_int res;
    mp_int mu;
    mp_digit buf;
    int     err, bitbuf, bitcpy, bitcnt, mode, digidx, x, y, winsize;
} LOCAL_VARS;

typedef struct _HISTORY_ELEMENT {
    ushort          lineNumber;
    ushort          pad;
    uint            crcLocalVars;
    uint            crcMpInt_WithoutDp; // mp_int structure excluding .dp
    uint            crcMpIntDp; // all mp_int's in LOCAL_VARS
    uint            crcRes; // just LOCAL_VARS.res
    uint            resDp[160]; // content of LOCAL_VARS.res
    ushort          bitbuf, bitcpy, bitcnt, mode, digidx, x, y;
} HISTORY_ELEMENT;

Here is the CPU info:
root@OpenWrt:/tmp# cat /proc/cpuinfo
processor       : 0
model name      : ARMv7 Processor rev 0 (v7l)
BogoMIPS        : 6.00
Features        : half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt 
vfpd32
CPU implementer : 0x51
CPU architecture: 7
CPU variant     : 0x2
CPU part        : 0x04d
CPU revision    : 0

processor       : 1
model name      : ARMv7 Processor rev 0 (v7l)
BogoMIPS        : 12.50
Features        : half thumb fastmult vfp edsp neon vfpv3 tls vfpv4 idiva idivt 
vfpd32
CPU implementer : 0x51
CPU architecture: 7
CPU variant     : 0x2
CPU part        : 0x04d
CPU revision    : 0

Hardware        : Generic DT based system
Revision        : 0000
Serial          : 0000000000000000

And the first few messages of the kernel log showing version and detected CPU 
details:

[    0.000000] Booting Linux on physical CPU 0x0
[    0.000000] Linux version 4.14.171 (builder@buildhost) (gcc version 7.5.0 
(OpenWrt GCC 7.5.0 r10947-65030d81f3)) #0 SMP Thu Feb 27 21:05:12 2020
[    0.000000] CPU: ARMv7 Processor [512f04d0] revision 0 (ARMv7), cr=10c5787d
[    0.000000] CPU: div instructions available: patching division code
[    0.000000] CPU: PIPT / VIPT nonaliasing data cache, PIPT instruction cache
[    0.000000] OF: fdt: Machine model: Netgear Nighthawk X4S R7800
[    0.000000] Memory policy: Data cache writealloc
[    0.000000] On node 0 totalpages: 122880
[    0.000000] free_area_init_node: node 0, pgdat c0a27880, node_mem_map 
dda39000
[    0.000000]   Normal zone: 960 pages used for memmap
[    0.000000]   Normal zone: 0 pages reserved
[    0.000000]   Normal zone: 122880 pages, LIFO batch:31
[    0.000000] random: get_random_bytes called from 0xc09008dc with crng_init=0
[    0.000000] percpu: Embedded 15 pages/cpu s29388 r8192 d23860 u61440
```
```
On Tue 24/3/2020, at 11:23 am, Horshack wrote:

I was able to isolate the issue to just a handful of assembly instructions 
within fast_s_mp_sqr(), related to the squaring loop. I broke that code out 
into a separate utility that reproduces the issue within a few seconds. The 
failure is somewhat sensitive to the data pattern and very sensitive to 
timing, indicating a likely memory/data path issue within my particular 
router. I'm guessing it's the IPQ8065 and not the SDRAM because I can get it 
to fail with a tiny data set easily fits within DCACHE. I can alter the 
frequency of the failure with a single ARM memory barrier instruction, which 
at first implied a superscalar data ordering condition but the memory barrier 
also alters the timing through the DCACHE so that is likely the effect it's 
having. I was able to exclude the VFP/Neon register corruption as the cause 
with some test code. I also excluded any context switch-speciifc issue by 
measuring the # of context switches in /proc/<pid>/status and catching a 
failure where no switches had occurred. I also modified the affinity so the 
utility runs on just one processor to rule out a specific core having the 
issue.

I put the source and binary of my utility on github - if anyone on this 
mailing list has this model router can you give it a try if possible? You 
only need the ipq8065-sqrbug (binary) and run-ipq8065-sqrbug.sh (script). 
Here's the link to the repository: 
https://github.com/horshack-dpreview/ipq8065-sqrbug 

```
