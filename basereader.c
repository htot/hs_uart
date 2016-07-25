#include "hs_serial.h"

static char state = WAIT_SOM;
static unsigned long STX_time;
void base_reader(mraa_uart_context uart, unsigned char * buffer, int *numBytes) { 
    int i, n, k;
    
    unsigned char base64buffer[3072];
    unsigned char readbuffer[3072];
    int bbpos = 0;
    struct _uart * u = uart;


    
    struct timespec Tick;
    unsigned long RelTime, TimePassed;
    
    if((k = getNumberOfAvailableBytes(u->fd)) > sizeof(readbuffer)) k = sizeof(readbuffer); 
    if(k>0) {
        if(clock_gettime(CLOCK_REALTIME, &Tick) != 0) return; // if we get no clock abort
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
                                                state = WAIT_SOM;
						break;
					case 0x03:
                                            // we found the end of the message, start decoding
                                            TimePassed = 1000000000L + RelTime - STX_time;
                                            TimePassed %= 1000000000L;
                                            if (TimePassed > 15000000L) {
                                                state = WAIT_SOM;
                                            } else {
						state = DECODED;
                                                toggle_gpio_value(1);
						n = base64_decode(buffer, base64buffer, bbpos);
                                                toggle_gpio_value(1);
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
			default:	// should only be DECODED
				break;
		};
	};
    };
    if(state != DECODED) {
        *numBytes = 0;
        return;
    } else {
        // we have decoded data so time remaining, now would be a good time for a nap
        *numBytes= n;
        state = WAIT_SOM;
    };
}
