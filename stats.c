#include "hs_serial.h"
#define MAX_EVENTS 1024

char * EventType[] = {
  "NO_CLOCK",
  "TRANSMIT",
  "STX_REC",
  "ETX_REC",
  "MISSED",
  "TIMEOUT",
  "OVERRUNS "
};
#define NUM_EVENTS sizeof(EventType) / sizeof(*EventType)

static struct timespec StartTime;
static double Event[MAX_EVENTS][NUM_EVENTS];

static signed EventNum = -1;
static signed Done = 0;

void StartTimer(void) {
    if(clock_gettime(CLOCK_REALTIME, &StartTime) != 0) {
        StartTime.tv_nsec = 0;
        StartTime.tv_sec = 0;
    };
    if(++EventNum == MAX_EVENTS) {
        Done = 1;
    } else if(EventNum > MAX_EVENTS) --EventNum;
}

void SignalEventsDone(void) {
    if(Done == 1) {
        fprintf(stderr, "Statistics Collected\n");
        Done = 0;
      }
}

void TimeEvent(char TheEvent) {
    struct timespec Now;

    if(EventNum < MAX_EVENTS) {
        if(clock_gettime(CLOCK_REALTIME, &Now) != 0) {
            Now.tv_nsec = 0;
            Now.tv_sec = 0;
            TheEvent = NO_CLOCK;
        };
        Event[EventNum][(unsigned)TheEvent] = (Now.tv_sec - StartTime.tv_sec) *1000 + (Now.tv_nsec - StartTime.tv_nsec) / (double)1e6;
    };
}

void PrintEvents(void) {
    unsigned i, j;

    printf("Event ");
    for(i = 0; i < NUM_EVENTS; i++) printf("%s ", EventType[i]);
    printf("\n");
    for(i = 0; i < EventNum; i++) {
        printf("%i ", (int)i);
        for(j = 0; j < NUM_EVENTS; j++) {
            printf("%8.3f ", Event[i][j]);
        };
        printf("\n");
    }
}
