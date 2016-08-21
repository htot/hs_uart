#include "hs_serial.h"

void dump_buffer(unsigned n, const unsigned char* buf) { //used for debugging
  unsigned i, j;

  for(i = 0; i < n; i += 16) {
    printf("%04i ", i);
    for(j = 0; j < 16; j++) {
      if(i + j < n) printf("%02x ", buf[i + j]);
      else printf("   ");
    };
    printf("\t");
    for(j = 0; j < 16; j++) {
      if(isprint(buf[i + j])) {
        if(i + j < n) printf("%c", buf[i + j]);
        else printf("   ");
      } else {
        if(i + j < n) printf(".");
        else printf("   ");
      };
    };
    printf("\n");
  };
}
