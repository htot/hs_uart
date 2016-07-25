#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mraa.h>
#include <sys/types.h>
#include <sys/time.h>

// FT need to link with librt (-lrt)
#include <time.h>

#include "encoding.h"

#define WAIT_SOM 0
#define WAIT_SOM_1 1
#define WAIT_SOM_2 2
#define READ 3
#define DECODED 4
#define TEST_GPIO 2
#define TEST_MODE 0
#define NUMBER_OF_GPIO 2

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
void base_reader(mraa_uart_context uart, unsigned char * buffer, int *numBytes);
void changemode(int dir);
int kbhit (void);
void init_gpio();
void toggle_gpio_value(int pin);
void set_gpio_value(int pin, int value);
int gpio_values[NUMBER_OF_GPIO];
mraa_result_t mraa_uart_set_non_blocking(mraa_uart_context dev, mraa_boolean_t nonblock);
void dump_buffer(unsigned n, const unsigned char* buf);
int getNumberOfAvailableBytes(int fd);
int set_interface_attribs (int fd, int speed, int parity, int disableFlowControl);
void set_blocking (int fd, int should_block);