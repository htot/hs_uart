#define _GNU_SOURCE 1
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include "crc32c.h"
#include "crc32intelc.h"
#include <endian.h>

// FT need to link with librt (-lrt)
#include <time.h>

#include "base64/include/libbase64.h"

#define WAIT_SOM 0
#define WAIT_SOM_1 1
#define WAIT_SOM_2 2
#define READ 3
#define DECODED 4
#define TEST_GPIO 2
#define TEST_MODE 0
#define NUMBER_OF_GPIO 2
#define MAX_BUFFER 4048
#define DATA_BUFFER 1024

#define NO_CLOCK 0
#define TRANSMIT 1
#define STX_REC  2
#define ETX_REC  3
#define MISSED   4
#define TIMEOUT  5
#define OVERRUNS 6
#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct _wtf mraa_adv_func_t;

/**
 * A structure representing a UART device
 */
struct _uart {
    /*@{*/
    int index; /**< the uart index, as known to the os. */
    const char* path; /**< the uart device path. */
    int fd; /**< file descriptor for device. */
    mraa_adv_func_t* advance_func; /**< override function table */
    /*@}*/
};


int main(int argc, char** argv);
int32_t Scan_Frame(const unsigned char * InBuffer, const size_t InLen, unsigned char * OutBuffer, size_t * OutLen, uint32_t *MessageNumber);
void changemode(int dir);
int kbhit (void);
void init_gpio();
void toggle_gpio_value(int pin);
void set_gpio_value(int pin, int value);
int gpio_values[NUMBER_OF_GPIO];
void dump_buffer(unsigned n, const unsigned char* buf);
int getNumberOfAvailableBytes(int fd);
int set_interface_attribs (int fd, int speed, tcflag_t parity, int disableFlowControl);
void set_blocking (int fd, int should_block);
int detect_rt(void);
void set_rt(void);
size_t FrameTransmitBuffer(unsigned char * TransmitBuffer, const uint32_t MessageNumber, const unsigned char * DataBuffer, const size_t n);
size_t UnframeReceiveBuffer(unsigned char * DataBuffer, uint32_t * MessageNumber, const unsigned char * ReceiveBuffer, const size_t n);
void StartTimer(void);
void TimeEvent(char Event);
void PrintEvents(void);
void exitmode(void);
void SignalEventsDone(void);
