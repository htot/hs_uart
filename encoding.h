/*
 * encoding.h
 *
 *  Created on: 5 Jun 2015
 *      Author: neil
 */

#ifndef ENCODING_H_
#define ENCODING_H_

unsigned char char2Hex(unsigned char c);
void char2Hex2(char * buf, unsigned char c);

int base64_encode(char *encoded, const char *src, int len);
int base64_decode(char *bufplain, const char *bufcoded, int len);

#endif /* ENCODING_H_ */
