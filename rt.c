#include "hs_serial.h"
#include <sys/utsname.h>
#include <sched.h>
#include <sys/mman.h>

#define MY_PRIORITY (49) /* we use 49 as the PREEMPT_RT use 50
                            as the priority of kernel tasklets
                            and interrupt handler by default */

#define MAX_SAFE_STACK (8*1024) /* The maximum stack size which is
                                   guaranteed safe to access without
                                   faulting */

#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */


int detect_rt(void) {
    struct utsname u;
    int crit = false;
    FILE *fd;

    uname(&u);
    if(strcasestr(u.version, "PREEMPT RT") == NULL) return false; /* uname contains PREEMPT RT */

    if ((fd = fopen("/sys/kernel/realtime","r")) != NULL) { /* /sys/kernel/realtime exists */
        int flag;
        crit = ((fscanf(fd, "%d", &flag) == 1) && (flag == 1)); /* and return 1 */
        fclose(fd);
    }
    return (crit);
}

static void stack_prefault(void) {

        unsigned char dummy[MAX_SAFE_STACK];

        memset(dummy, 0, MAX_SAFE_STACK);
        return;
}

void set_rt(void) {
        struct sched_param param;

        /* Declare ourself as a real time task */

        param.sched_priority = MY_PRIORITY;
        if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
                perror("sched_setscheduler failed");
                exit(-1);
        }

        // Lock memory

        if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
                perror("mlockall failed");
                exit(-2);
        }

        // Pre-fault our stack

        stack_prefault(); 
}
