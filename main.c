#include "hs_serial.h"

int main(int argc, char** argv)
{
    int i, j, written, numofbytes, n, fd;
    unsigned char writebuffer[2060];
    unsigned char textbuffer[2048];
    unsigned char readbuffer[3072];
    
    // FT
    struct timespec Tick;
    unsigned long TimeNow, TimeLast, TimePassed;
    
    changemode(1); //configure keyboard to not wait for enter
     
    
    //fill textbuffer with 0/0xff repeated
    for(i=0;i<sizeof(textbuffer);i++){
        textbuffer[i] = (char)i;
    }
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

    
 //   mraa_uart_set_baudrate(uart,2000000);
 //   mraa_uart_set_non_blocking(uart,1);
 // FT get tick time

    clock_gettime(CLOCK_REALTIME, &Tick); //retrieve time of realtime clock
    TimeLast = (unsigned long)Tick.tv_nsec;
        
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
        usleep(500);
    }
    changemode(0); //reset keyboard default behaviour
    mraa_uart_stop(uart); // stop uart
    mraa_deinit(); //stop mraa
//    	close(fd); 
    
}