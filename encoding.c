unsigned char char2Hex(unsigned char c) {
  switch(c) {
    case  0: return '0';
    case  1: return '1';
    case  2: return '2';
    case  3: return '3';
    case  4: return '4';
    case  5: return '5';
    case  6: return '6';
    case  7: return '7';
    case  8: return '8';
    case  9: return '9';
    case 10: return 'A';
    case 11: return 'B';
    case 12: return 'C';
    case 13: return 'D';
    case 14: return 'E';
    case 15: return 'F';
    default: return '0';
  }
}

void char2Hex2(char * buf, unsigned char c) {
  *buf = char2Hex(c >> 4);
  *(buf+1) = char2Hex(c & 0x0F);
}

static const char basis_64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//int base64_encode_len(int len) {
//  return ((len + 2) / 3 * 4);
//}

int base64_encode(char *encoded, const char *src, int len) {
  int i;
  char *p;

  p = encoded;
  for (i = 0; i < len - 2; i += 3) {
    *p++ = basis_64[(src[i] >> 2) & 0x3F];
    *p++ = basis_64[((src[i] & 0x3) << 4) |
                    ((int) (src[i + 1] & 0xF0) >> 4)];
    *p++ = basis_64[((src[i + 1] & 0xF) << 2) |
                    ((int) (src[i + 2] & 0xC0) >> 6)];
    *p++ = basis_64[src[i + 2] & 0x3F];
  }
  if (i < len) {
    *p++ = basis_64[(src[i] >> 2) & 0x3F];
    if (i == (len - 1)) {
      *p++ = basis_64[((src[i] & 0x3) << 4)];
      *p++ = '=';
    }
    else {
      *p++ = basis_64[((src[i] & 0x3) << 4) |
                      ((int) (src[i + 1] & 0xF0) >> 4)];
      *p++ = basis_64[((src[i + 1] & 0xF) << 2)];
    }
    *p++ = '=';
  }

  //*p++ = '\0';

  return p - encoded;
}

static const unsigned char pr2six[256] = {
    /* ASCII table */
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

//int base64_decode_len(int len) {
//  return ((len + 3)/4) * 3;
//}

int base64_decode(char *bufplain, const char *bufcoded, int len) {
  int nbytesdecoded;
  register const unsigned char *bufin;
  register unsigned char *bufout;

  // wj: skip padding
  if(bufcoded[len-1] == '=')
    len--;
  if(bufcoded[len-1] == '=')
    len--;

  nbytesdecoded = ((len + 3) / 4) * 3;
  bufout = (unsigned char *) bufplain;
  bufin = (const unsigned char *) bufcoded;

  while (len > 4) {
    *(bufout++) =
        (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    *(bufout++) =
        (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    *(bufout++) =
        (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    bufin += 4;
    len -= 4;
  }

  /* Note: (len == 1) would be an error, so just ingore that case */
  // wj: this seems to assume there are no padding chars
  if (len > 1) {
    *(bufout++) =
        (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
  }
  if (len > 2) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
  }
  if (len > 3) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
  }

  //*(bufout++) = '\0';

  nbytesdecoded -= (4 - len) & 3;

  return nbytesdecoded;
}

/*
static char * tstencoded =
  "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIj"
  "JCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZH"
  "SElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWpr"
  "bG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6P"
  "kJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKz"
  "tLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX"
  "2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7"
  "/P3+/w==";
#include <stdio.h>
int main() {
  int i, j, k, l;
  unsigned char in[256];
  unsigned char out[512];
  unsigned char back[512];

  // create test data
  for(i=0;i<256;i++)
    in[i] = i;

  // encode
  l = base64_encode(out, in, 256);
  printf("encoded length: %d\n", l);
  for(i=0;i<l;i++) {
    if(i % 48 == 0)
      printf("\n");
    putc(out[i], stdout);
  }
  printf("\n");

  // decode back
  l = base64_decode(back, out, l);
  printf("decoded length: %d\n", l);
  for(i=0;i<l;i++) {
    if(i % 32 == 0)
      printf("\n");
    printf("%02X ", back[i]);
  }
}
*/
