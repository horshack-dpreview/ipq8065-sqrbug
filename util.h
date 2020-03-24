#ifndef _INCL_UTIL
#define _INCL_UTIL

#define countof(x)  (sizeof(x) / sizeof((x)[0]))

extern int get_context_switch_counts(int *voluntary, int *involuntary);
extern int set_cpu_affinity(uint32_t cpuMask);
extern void hex_dump_32bit(const char *desc, void *addr, int bytes, int wordToHighlight);
extern void hex_dump_64bit(const char *desc, void *addr, int bytes, int wordToHighlight);

#endif /* #ifndef _INCL_UTIL */
