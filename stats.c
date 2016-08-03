#include "hs_serial.h"

char * EventType[] = {
  "NO_CLOCK",
  "TRANSMIT",
  "STX_REC",
  "ETX_REC",
  "MISSED",
  "TIMEOUT",
  "OVERRUNS "
};

static struct timespec StartTime;
static struct Events {
    unsigned Event;
    struct timespec EventTime;
} Event[1024];
static unsigned EventNum = 0, LastEvent = 0;

void StartTimer(void) {
    if(clock_gettime(CLOCK_REALTIME, &StartTime) != 0) {
        StartTime.tv_nsec = 0;
        StartTime.tv_sec = 0;
    };
}

void TimeEvent(char TheEvent) {
    struct timespec EventTime;

    if(clock_gettime(CLOCK_REALTIME, &EventTime) != 0) {
        EventTime.tv_nsec = 0;
        EventTime.tv_sec = 0;
        TheEvent = NO_CLOCK;
    };
    Event[EventNum].Event = TheEvent;
    Event[EventNum].EventTime.tv_sec = EventTime.tv_sec - StartTime.tv_sec;
    Event[EventNum].EventTime.tv_nsec = EventTime.tv_nsec - StartTime.tv_nsec;
    if(EventNum > LastEvent) LastEvent = EventNum;
    if(++EventNum >= 1024) EventNum = 0;
}

void PrintEvents(void) {
    unsigned i;

    for(i = 0; i <= LastEvent; i++) {
        printf("%i %s %lu.%06lu\n", i, EventType[Event[i].Event], Event[i].EventTime.tv_sec, Event[i].EventTime.tv_nsec / 1000);
    }

}
