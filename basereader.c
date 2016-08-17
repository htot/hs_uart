#include "hs_serial.h"

static char state = WAIT_SOM;
static unsigned long STX_time;
static unsigned char base64buffer[3072];
static int bbpos = 0;

int32_t base_reader(const mraa_uart_context uart, unsigned char * buffer, uint32_t *MessageNumber) {
    int j, i, k;
    
    unsigned char readbuffer[3072];
    struct _uart * u = uart;
    int32_t numBytes = -1;

    
    struct timespec Tick;
    unsigned long RelTime, TimePassed;
    
    if((k = getNumberOfAvailableBytes(u->fd)) > sizeof(readbuffer)) k = sizeof(readbuffer); 
    if(k>0) {
        if(clock_gettime(CLOCK_REALTIME, &Tick) != 0) return -1; // if we get no clock abort
        mraa_uart_read(uart, readbuffer, k);
        RelTime = (unsigned long)Tick.tv_nsec;
	// check the new bytes
        for(i=0;i<k;i++) {
 		switch(state) {
			case WAIT_SOM :
                                
				// a message starts with at least 1 PREAMBLE
				if(readbuffer[i] == 0xFF){
                                    state = WAIT_SOM_1;
				};
				break;
			case WAIT_SOM_1 :
				if(readbuffer[i] == 0x02) {
                                    // we found STX 
                                    STX_time = RelTime;
                                    state = READ;
                                    TimeEvent(STX_REC);
                                    toggle_gpio_value(1);
                                    bbpos = 0;
				} else if(readbuffer[i] != 0xFF) {
					// we found something else, wait for new SOM
					state = WAIT_SOM;
				};
				break;
			case READ :
				switch(readbuffer[i]) {
					case 0xFF: 
                                            // found a new preamble, go back to wait for STX
                                            state = WAIT_SOM_1;
                                            break;
					case 0x00:
						// looks like we found an error, go back to wait for PREAMBLE
                                                printf ("PE\n");
                                                state = WAIT_SOM;
						break;
					case 0x03:
                                            // we found the end of the message, start decoding
                                            TimePassed = 1000000000L + RelTime - STX_time;
                                            TimePassed %= 1000000000L;
                                            if (TimePassed > 15000000L) {
                                                TimeEvent(TIMEOUT);
                                                state = WAIT_SOM;
                                                break;
					    };
					    TimeEvent(ETX_REC);
					    if((numBytes = UnframeReceiveBuffer(buffer, MessageNumber, base64buffer, bbpos)) < 0) {
						state = WAIT_SOM;
					    } else {
                                                toggle_gpio_value(1);
                                                state = WAIT_SOM;     // we have numBytes but can start decoding next message
                                            };
                                            break;
					default:
                                            // we found a normal char, copy to base64buffer
                                            TimePassed = 1000000000L + RelTime - STX_time;
                                            TimePassed %= 1000000000L;
                                            if (TimePassed > 15000000L) {
                                                state = WAIT_SOM;
                                                break;  // abort processing the buffer
                                            } else {
						base64buffer[bbpos++] = readbuffer[i];
						if(bbpos >= sizeof(base64buffer)) { // we are overflowing the buffer, start waiting for STX
                                                    state = WAIT_SOM;
                                                    printf("  !!overflow\n");
                                                    break; // abort processing the buffer
                                                };
                                            };	
                                            break;
				};
				break;
			default:	// should never be here
				break;
		};
	};
    };
    return numBytes;                  // numBytes is always -1 unless data is available in buffer
}
