#include "hs_serial.h"

static char state = WAIT_SOM;
static struct timespec STX_time;
static unsigned char LocalBuffer[2 * MAX_BUFFER];
static size_t LocalBufferTail = 0, STXPos = 0, LastPos = 0;

extern int DebugFlag;
extern unsigned long msecs;

// Scan_Frame takes InBuffer and appends that to LocalBuffer. It then scans for the first Frame.
// A Frame consists of PREAMBLES, STX, base64 encoded (MSG#, Size, Data, CRC), ETX. Everything
// between STX and ETX is Unframed, MessageNumber and Data (in OutBuffer) stored, Size returned
// and LocalBuffer updated. More messages may be pending in LocalBuffer, so Scan_Frame should be
// repeatedly called until nothing is found.
// If nothing is found -1 is returned.
int Scan_Frame(const char * InBuffer, const size_t InLen, unsigned char * OutBuffer, size_t * OutLen, uint32_t *MessageNumber)
{
        size_t i, j, NumBytes = -1;
        int32_t Received;
        int Found;

        struct timespec Tick;
        unsigned long TimePassed;

        // Append InBuffer to LocalBuffer
        for(i = 0; i < InLen; i++) LocalBuffer[LocalBufferTail + i] = InBuffer[i];
        LocalBufferTail += InLen;

        if (LocalBufferTail > 0) 
        {
                // Calculate the time passed since the last STX found
                if (clock_gettime(CLOCK_REALTIME, &Tick) != 0) 
                {
                        if (DebugFlag) fprintf(stderr, "No clock tick on receive\n");
                        return -1; // if we get no clock abort for now and retry later
                };
                TimePassed = (Tick.tv_sec - STX_time.tv_sec) * 1000 + (Tick.tv_nsec - STX_time.tv_nsec) / 1000000; // in msec

                // check the new bytes
                for(i = LastPos; i < LocalBufferTail; i++) 
                {
                        switch(state) 
                        {
                                case WAIT_SOM :
                                // a message starts with at least 1 PREAMBLE
                                if(LocalBuffer[i] == 0xFF)
                                {
                                        state = WAIT_SOM_1;
                                };
                                break;
                
                                case WAIT_SOM_1 :
                                if(LocalBuffer[i] == 0x02)
                                {
                                        // we found STX
                                        STX_time = Tick;
                                        TimePassed = 0; //  found a new STX, reset TimePassed
                                        state = READ;
                                        TimeEvent(STX_REC);
                                        STXPos = i;
                                } else if(LocalBuffer[i] != 0xFF)
                                {
                                        // we found something else, wait for new SOM
                                        if (DebugFlag) fprintf(stderr, "Unexpected char while waiting for STX\n");
                                        state = WAIT_SOM;
                                };
                                break;

                                case READ :
                                switch(LocalBuffer[i]) 
                                {
                                        case 0xFF :
                                        // found a new preamble, go back to wait for STX
                                        if (DebugFlag) fprintf(stderr, "Received FF\n");
                                        state = WAIT_SOM_1;
                                        break;
                        
                                        case 0x00 :
                                        // looks like we found an error, go back to wait for PREAMBLE
                                        if (DebugFlag) fprintf(stderr, "Received NULL\n");
                                        state = WAIT_SOM;
                                        break;

                                        case 0x03 :
                                        // we found the end of the message, start decoding
                                        if (TimePassed > msecs - 1) 
                                        {
                                                TimeEvent(TIMEOUT);
                                                if (DebugFlag) fprintf(stderr, "Received ETX too late\n");
                                                state = WAIT_SOM;
                                                Found = -1;
                                                *OutLen = 0;
                                        } else if((Received = UnframeReceiveBuffer(OutBuffer, MessageNumber, LocalBuffer + STXPos + 1, i - STXPos - 1)) < 0) 
                                        {
                                                Found = -1;
                                                *OutLen = 0;
                                        } else {
                                                *OutLen = Received;
                                                Found = 0;
                                                TimeEvent(ETX_REC);
                                                LocalBufferTail -= i;
                                                LocalBufferTail--;
                                                for(j = 0; j < LocalBufferTail; j++) LocalBuffer[j] = LocalBuffer[i + 1 + j];
                                                state = WAIT_SOM;
                                                LastPos = 0;
                                                return(Found);
                                        };
                                        // decode failed, continue
                                        state = WAIT_SOM;
                                        break;

                                        default :
                                        // we found a normal char ,do nothing
                                        break;
                                };
                                break;
                        };
                };

        };
        LastPos = LocalBufferTail;
        *OutLen = 0;
        return -1;
}
