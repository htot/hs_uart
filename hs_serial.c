#include "hs_serial.h"

int getNumberOfAvailableBytes(int fd) {
  int nbytes;
  ioctl(fd, FIONREAD, &nbytes);
  return nbytes;
}

int
set_interface_attribs (int fd, int speed, tcflag_t parity, int disableFlowControl)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        tty.c_iflag |= IGNBRK;          // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 0;            // no timeout
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls, enable UART
        tty.c_cflag &= ~CSTOPB;
        
        if(parity == 0) {
            tty.c_iflag |= IGNPAR;         // do not ignore parity and framing errors
            tty.c_iflag |= PARMRK;         // parity or framing error char replaced with \0
            tty.c_iflag &= ~INPCK;           // enable parity checking
            tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity          
        } else {
            tty.c_iflag &= ~IGNPAR;         // do not ignore parity and framing errors
            tty.c_iflag |= PARMRK;         // parity or framing error char replaced with \0
            tty.c_iflag |= INPCK;           // enable parity checking
            tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
            tty.c_cflag |= parity;
        };
        
        if(disableFlowControl)
          tty.c_cflag &= ~CRTSCTS; // no rts/cts
        else
          tty.c_cflag |= CRTSCTS; // enable rts/cts
//          mraa_intel_edison_pinmode_change(128, 1); // CTS
//          mraa_intel_edison_pinmode_change(129, 1); // RTS
//          mraa_uart_set_flowcontrol()
        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                printf ("error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}


void
set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 0;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                printf ("error %d setting term attributes", errno);
}
