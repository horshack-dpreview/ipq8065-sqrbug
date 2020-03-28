#include "includes.h"

#define _GNU_SOURCE
#include <sched.h>

/*
 * Returns the number of context switches for this task, both
 * voluntary and involuntary counts
*/ 
int get_context_switch_counts(int *voluntary, int *involuntary) {

    int     pid = getpid();
    int     ctxSwitchCount;
    size_t  bytesRead;
    char    filename[64];
    char    *p, buff[4096];
    char    scanfbuff[80];

    sprintf(filename, "/proc/%d/status", pid);
    FILE *f = fopen(filename, "r");
    if (f != NULL) {
        bytesRead = fread(buff, 1, sizeof(buff), f);
        fclose(f);
        if (bytesRead) {
            if ((p = strstr(buff,"voluntary_ctxt_switches")) != NULL) {
                if (sscanf(p, "%s%d", scanfbuff, &ctxSwitchCount)) {
                    *voluntary = ctxSwitchCount;
                    if ((p = strstr(buff,"nonvoluntary_ctxt_switches")) != NULL)
                        if (sscanf(p, "%s%d", scanfbuff, &ctxSwitchCount)) {
                            *involuntary = ctxSwitchCount;
                            return 0;
                        }
                }
            }
        }
    }
    return -1;
}

/*
 *
 * Sets the CPU affinity for this task. 'cpuMask' has a bit
 * set corresponding to each CPU this processed is allowed
 * to run on
 */
int set_cpu_affinity(uint32_t cpuMask) {

    pid_t       pid = getpid();
    cpu_set_t   cpuSet;

    if (sched_getaffinity(pid, sizeof(cpuSet), &cpuSet) == 0) {
        cpuSet.__bits[0] = cpuMask;
        if (sched_setaffinity(pid, sizeof(cpuSet), &cpuSet)==0)
            return 0;
        else
            printf("sched_setaffinity() failed\n");
    } else
        printf ("sched_getaffinity() failed\n");
    return 1;
}

/*
 * Dumps a block of memory as 32-bit hex words
 */
void hex_dump_32bit(const char *desc, void *addr, int bytes, int wordToHighlight) {
    
    int			wordIndex;
    uint32_t	*p = (uint32_t*)addr;

    printf ("%s:\n", desc);
    for (wordIndex = 0; wordIndex < bytes/4; wordIndex++) {
        if ((wordIndex % 8) == 0) {
            if (wordIndex != 0)
                printf("\n");
            printf("  %04x ", wordIndex*sizeof(uint32_t)); 
        }   
        if (wordIndex % 8 == 4)
            printf(" -");
        if (wordIndex == wordToHighlight)
            printf("*%08x", p[wordIndex]);
        else
            printf(" %08x", p[wordIndex]);
    }   
    printf("\n");
    fflush(stdout);
}

