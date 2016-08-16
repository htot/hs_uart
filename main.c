#include "hs_serial.h"

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char** argv)
{
    int c, i, j, written, numofbytes, n, UartFd, TimerFd, KbHit, MaxFd;
    int TransmitSize, MessageNumber = 0;
    int ReceiveMessageNumber, PreviousReceiveMessageNumber = 0;
    unsigned char writebuffer[MAX_BUFFER];
    unsigned char textbuffer[DATA_BUFFER];
    unsigned char readbuffer[3072];
    struct timespec Tick;
    unsigned long TimeNow, TimeLast, TimePassed;
    struct itimerspec TimerSettings;
    uint64_t TimerValue;
    fd_set active_rfds, read_rfds;
    tcflag_t parity = (PARENB | PARODD);

    
    
     while((c = getopt (argc, argv, "e?:")) != -1) {
        switch (c) {
            case 'e':
                parity = PARENB;
                printf("Parity set to even\n");
                break;
            case '?':
                printf("Usage: %s [-e]\n", argv[0]);
                printf("  -e : switch to parity even\n");
                return;
            default:
                fprintf(stderr, "Usage: %s [-e]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    changemode(1); //configure keyboard to not wait for enter
     
    //fill textbuffer with random
    for(i=0;i<DATA_BUFFER;i++){
        textbuffer[i] = (char)i;
    };

    mraa_init(); //initialize mraa
    init_gpio(); // initialize gpio pins
    mraa_uart_context uart;
    uart = mraa_uart_init(0);
    mraa_uart_flush(uart);
    
    struct _uart * u = uart;
    UartFd = u->fd;
  // B1000000 
  // B1152000
  // B1500000
  // B2000000
  // B2500000
  // B3000000
  // B3500000
  // B4000000 crashes edison!

    set_interface_attribs(UartFd, B2000000, parity , 1);  //set serial port to 8 bits, 2Mb/s, parity ODD, 1 stop bit
    set_blocking(UartFd, 0);                                        //set serial port non-blocking

    // Create a CLOCK_REALTIME relative timer with initial expiration 1 sec. and interval 15msec. */
    TimerSettings.it_value.tv_sec = 1;
    TimerSettings.it_value.tv_nsec = 0;
    TimerSettings.it_interval.tv_sec = 0;
    TimerSettings.it_interval.tv_nsec = 15000000L;

    if((TimerFd = timerfd_create(CLOCK_REALTIME, 0)) == -1) handle_error("timerfd_create");
    if(timerfd_settime(TimerFd, 0, &TimerSettings, NULL) == -1) handle_error("timerfd_settime");

    // find highest fd
    FD_ZERO(&active_rfds);                                       // clears the set
    FD_SET(STDIN_FILENO, &active_rfds);                          // add stdin to the set
    FD_SET(TimerFd, &active_rfds);                               // add timer to the set
    FD_SET(UartFd, &active_rfds);                                // add uart to the set
    MaxFd = 0;
    if(STDIN_FILENO > MaxFd) MaxFd = STDIN_FILENO;
    if(TimerFd > MaxFd) MaxFd = TimerFd;
    if(UartFd > MaxFd) MaxFd = UartFd;

    if(detect_rt()) {
        printf("preempt_rt detected\n");
        set_rt();                                             // on preempt_rt we need to set priority and lock memory
    };

    KbHit = 0;
    while ( !KbHit){                                          // send/receive data continuously until kbhit
        read_rfds = active_rfds;
        if(select(MaxFd+1, &read_rfds, NULL, NULL, NULL) < 0) handle_error("select"); // wait indefinetely
        if(FD_ISSET(STDIN_FILENO, &read_rfds)) {
            KbHit = 1;                                        // the kb has been pressed
            FD_SET(STDIN_FILENO, &active_rfds);
        };
        if(FD_ISSET(TimerFd, &read_rfds)) {                        // a time tick happened
            if(read(TimerFd, &TimerValue, sizeof(uint64_t)) != sizeof(uint64_t)) handle_error("read timer");
            toggle_gpio_value(0);
            MessageNumber++;
            StartTimer();
            TransmitSize = FrameTransmitBuffer(writebuffer, MessageNumber, textbuffer, DATA_BUFFER);
            if(mraa_uart_write(uart, writebuffer, TransmitSize) != TransmitSize) handle_error("mraa_uart_write");; //write data into the uart buffer non blocking
            TimeEvent(TRANSMIT);
            toggle_gpio_value(0);
            FD_SET(TimerFd, &active_rfds);
        };
        if(FD_ISSET(UartFd, &read_rfds)) {                         // data is in the Uart
            if(base_reader(uart, readbuffer, &ReceiveMessageNumber) >= 0) {
                if(ReceiveMessageNumber != PreviousReceiveMessageNumber + 1) {
                    TimeEvent(MISSED);
                };
                PreviousReceiveMessageNumber = ReceiveMessageNumber;
            };
            FD_SET(UartFd, &active_rfds);
        };
    }
    changemode(0);                                            // reset keyboard default behaviour
    mraa_uart_stop(uart);                                     // stop uart
    mraa_deinit();                                            // stop mraa
    PrintEvents();
}
