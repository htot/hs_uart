#include "hs_serial.h"

void dump_buffer(unsigned n, const unsigned char* buf) { //used for debugging
  int on_this_line = 0;
  while (n-- > 0) {
    fprintf(stdout, "%02X ", *buf++);
    on_this_line += 1;
    if (on_this_line == 32 || n == 0) {
      int i;
      fputs(" ", stdout);
      for (i = on_this_line; i < 32; i++)
        fputs("   ", stdout);
      for (i = on_this_line; i > 0; i--)
        fputc(isprint(buf[-i]) ? buf[-i] : '.', stdout);
      fputs("\n", stdout);
      on_this_line = 0;
    }
  }
}
