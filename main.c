#include "hs_serial.h"

int main(int argc, char** argv)
{
    int i, j, written, numofbytes, n, fd;
    unsigned char writebuffer[2060];
    unsigned char textbuffer[2048];
    unsigned char readbuffer[3072];
    uint32_t CRC32C, * p_crc;
    struct timespec Tick;
    unsigned long TimeNow, TimeLast, TimePassed;

    changemode(1); //configure keyboard to not wait for enter
     
    
    //fill textbuffer with 0/0xff repeated
    for(i=0;i<sizeof(textbuffer);i++){
        textbuffer[i] = (char)i;
    };
    // put 4 zero at the last 1024 bytes
    p_crc = (uint32_t *)(textbuffer + 1020);
    *p_crc = 0;
    
    // calculate the CRC
    CRC32C = crc32cIntelC (crc32cInit(), textbuffer, 1024);
    CRC32C = crc32cFinish(CRC32C);
    printf("Buffer CRC32C = 0x%08x\n", CRC32C);
    // put the CRC at the end of the buffer
    *p_crc = htole32(CRC32C);	// write in little endian
    
    unsigned int pos = 0; //message sent
    writebuffer[pos++] = 0xFF;
    writebuffer[pos++] = 0xFF;
    writebuffer[pos++] = 0x02;
    pos += base64_encode(&writebuffer[pos], textbuffer, 1024);
    writebuffer[pos++] = 0x03;

    mraa_init(); //initialize mraa
    init_gpio(); // initialize gpio pins
    mraa_uart_context uart;
    uart = mraa_uart_init(0);
    mraa_uart_flush(uart);
    
    struct _uart * u = uart;
    fd = u->fd;
  // B1000000 
  // B1152000
  // B1500000
  // B2000000
  // B2500000
  // B3000000
  // B3500000
  // B4000000 crashes edison!

    set_interface_attribs(fd, B2000000, (PARENB | PARODD), 1); //set serial port to 8 bits, 2Mb/s, parity ODD, 1 stop bit
    set_blocking(fd, 0); //set serial port non-blocking

    if(clock_gettime(CLOCK_REALTIME, &Tick) != 0) { //retrieve time of realtime clock
        perror("We didn't get a time\n");
        exit(-1);
    };
    TimeLast = (unsigned long)Tick.tv_nsec;
    TimePassed = 0;

    if(detect_rt()) {
        printf("preempt_rt detected\n");
        set_rt(); // on preempt_rt we need to set priority and lock memory
    };
       
    while ( !kbhit()){ //send/resend data continuously
        if(clock_gettime(CLOCK_REALTIME, &Tick) == 0) { //retrieve time of realtime clock
            TimeNow = (unsigned long)Tick.tv_nsec;
            TimePassed = 1000000000L + TimeNow - TimeLast;
            TimePassed %= 1000000000L;
        };
        if (TimePassed > 15000000L) {
            toggle_gpio_value(0);
            written = mraa_uart_write(uart, writebuffer, pos); //write data into the uart buffer non blocking
            toggle_gpio_value(0);
            TimeLast = TimeNow;
            TimePassed = 0;
        };
        base_reader(uart, readbuffer, &n); //read and decode data from the uart buffer before deadline
        usleep(100);
    }
    changemode(0); //reset keyboard default behaviour
    mraa_uart_stop(uart); // stop uart
    mraa_deinit(); //stop mraa
//    	close(fd); 
    
}
