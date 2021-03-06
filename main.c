#include "hs_serial.h"
#include <mraa.h>
mraa_result_t mraa_uart_set_non_blocking(mraa_uart_context dev, mraa_boolean_t nonblock);

#define USAGE "Usage: %s [-b nnnn] [-d] [-e] [-f] [-n] [-t nnnn] [-s] [-r nnnn]\n"

int DebugFlag = 0;
long msecs = 15;


int main(int argc, char** argv)
{
    int c, i, k, stats=0, UartFd, TimerFd, KbHit, MaxFd, FlowControl = 0, BaudRate = 2000000;
    int BufferSize = 1024;
    size_t DecodeSize;
    int TransmitSize;
    uint32_t ReceiveMessageNumber, PreviousReceiveMessageNumber = 0, MessageNumber = 0;
    unsigned char writebuffer[MAX_BUFFER];
    unsigned char textbuffer[DATA_BUFFER];
    unsigned char decodebuffer[3072], readbuffer[3072];
    struct itimerspec TimerSettings;
    uint64_t TimerValue;
    fd_set active_rfds, read_rfds;
    tcflag_t parity = (PARENB | PARODD);

     while((c = getopt (argc, argv, "b:defn?t:sr:")) != -1) {
        switch (c) {
          case 'b':
              BufferSize = -1;
              BufferSize = atoi(optarg);
              if (BufferSize == -1) {
                  fprintf(stderr, USAGE, argv[0]);
                  exit(EXIT_FAILURE);
              }
              fprintf(stderr, "set message size to %i\n", (int)BufferSize);
              break;
          case 'd':
              DebugFlag = 1;
              fprintf(stderr, "Debug on\n");
              break;
            case 'e':
                parity = PARENB;
                fprintf(stderr, "Parity set to even\n");
                break;
            case 'f':
                FlowControl = 1;
                fprintf(stderr, "RTS/CTS flow control ON\n");
            break;
            case 'n':
                parity = 0;
                fprintf(stderr, "Parity set to none\n");
                break;
            case 't':
                msecs=-1;
                msecs = atoi(optarg);
                if (msecs == -1) {
                    fprintf(stderr, USAGE, argv[0]);
                    exit(EXIT_FAILURE);
                }
                fprintf(stderr, "set timer to %i ms\n", (int)msecs);
                break;
            case '?':
                printf(USAGE, argv[0]);
                printf("  -b nnnn : set buffer size (default = 1024)\n");
                printf("  -d      : Print debug messages to screen, may require -t 1000 on slow terminals\n");
                printf("  -e      : Switch to parity even\n");
                printf("  -f      : Turn off RTS/CTS flow control\n");
                printf("  -n      : Switch to parity none\n");
                printf("  -t nnnn : msec between transmits\n");
                printf("  -r nnnn : Set buad rate\n");
                printf("  -s      : Print statistcs when done\n");
                printf("\nPress any key to terminate\n");
                exit(0);
            case 's':
                stats = 1;
                fprintf(stderr, "Print statistcs when done\n");
                break;
            case 'r':
                BaudRate = -1;
                BaudRate = atoi(optarg);
                if (BaudRate == -1) {
                    fprintf(stderr, USAGE, argv[0]);
                    exit(EXIT_FAILURE);
                }
                fprintf(stderr, "Set buad rate to %i\n", (int)BaudRate);
            break;

            default:
                fprintf(stderr, USAGE, argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    atexit(exitmode);
    changemode(1); //configure keyboard to not wait for enter
     
    //fill textbuffer with random
    for(i=0;i<DATA_BUFFER;i++){
        textbuffer[i] = (char)i;
    };
    if (DebugFlag) fprintf(stderr, "frame size is %i\n", (int)FrameTransmitBuffer(writebuffer, MessageNumber, textbuffer, BufferSize));

    if(mraa_init() != MRAA_SUCCESS) {
        fprintf(stderr, "MRAA failed to initialize\n");
        return EXIT_FAILURE;
    }

    init_gpio(); // initialize gpio pins

    mraa_uart_context uart;
    uart = mraa_uart_init(0);
    if (uart == NULL) {
        fprintf(stderr, "UART failed to setup\n");
        return EXIT_FAILURE;
    }

    struct _uart * u = uart;
    UartFd = u->fd;
  // valid B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000
  // B4000000 crashes edison!

    mraa_uart_set_baudrate(uart, (unsigned int)BaudRate);
    switch(parity) {
    case 0 :
        mraa_uart_set_mode(uart, 8, MRAA_UART_PARITY_NONE, 1);
        break;
    case (PARENB | PARODD):
        mraa_uart_set_mode(uart, 8, MRAA_UART_PARITY_ODD, 1);
        break;
    case PARENB :
        mraa_uart_set_mode(uart, 8, MRAA_UART_PARITY_EVEN, 1);
        break;
    }
    mraa_uart_set_flowcontrol(uart, 0, FlowControl);
    mraa_uart_set_non_blocking(uart, 1);
    mraa_uart_flush(uart);

    // Create a CLOCK_REALTIME relative timer with initial expiration 1 sec. and interval 15msec. */
    TimerSettings.it_value.tv_sec = 1;
    TimerSettings.it_value.tv_nsec = 0;
    TimerSettings.it_interval.tv_sec = msecs / 1000;
    TimerSettings.it_interval.tv_nsec = (msecs % 1000) * 1000000;

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
        fprintf(stderr, "preempt_rt detected\n");
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
            if(stats == 1) SignalEventsDone();
            TransmitSize = FrameTransmitBuffer(writebuffer, MessageNumber, textbuffer, BufferSize);
            if(mraa_uart_write(uart, (char *)writebuffer, TransmitSize) != TransmitSize) handle_error("mraa_uart_write");; //write data into the uart buffer non blocking
            TimeEvent(TRANSMIT);
            toggle_gpio_value(0);
            FD_SET(TimerFd, &active_rfds);
        };
        if(FD_ISSET(UartFd, &read_rfds)) {                         // data is in the Uart
            if ((k = mraa_uart_read(uart, (char *)readbuffer, sizeof(readbuffer))) == -1) {
                if (DebugFlag) fprintf(stderr, "Reading the buffer did not succeed\n");
                return -1;
            };
            while(Scan_Frame(readbuffer, k, decodebuffer, &DecodeSize, &ReceiveMessageNumber) >= 0) {
                k = 0;
                if(ReceiveMessageNumber != PreviousReceiveMessageNumber + 1) {
                    TimeEvent(MISSED);
                    if (DebugFlag) fprintf(stderr, "Received message %i expected %i\n", (int)ReceiveMessageNumber, (int)PreviousReceiveMessageNumber + 1);
                }
                PreviousReceiveMessageNumber = ReceiveMessageNumber;
            };
            FD_SET(UartFd, &active_rfds);
        };
    }
    changemode(0);                                            // reset keyboard default behaviour
    mraa_uart_stop(uart);                                     // stop uart
    mraa_deinit();                                            // stop mraa
    if(stats == 1) PrintEvents();
}
