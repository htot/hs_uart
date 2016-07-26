#include "crc32c.h"
#include "crc32intelc.h"
#include <stdio.h>
#include <endian.h>

#define BUFFER_SIZE 2048

int main(int argc, char** argv)
{
    unsigned char buffer[BUFFER_SIZE + 10], string[] = "1234567890";
    uint32_t CRC32C, * p_crc, R_CRC32C;
    char * p_buf = buffer;
    
    CRC32C = crc32cIntelC (crc32cInit(), string, sizeof(string) - 1);
    CRC32C = crc32cFinish(CRC32C);
    printf("string CRC32C = 0x%08x, known good value 0xf3dbd4fe\n", CRC32C); /* 0xf3dbd4fe is the correct value */

    
    // fill the buffer with non-zero data
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        buffer[i] = (char) i;
    };
    // put 4 zero at the end
    p_crc = (uint32_t *)&buffer[BUFFER_SIZE];
    *p_crc = 0;
    
    // calculate the CRC
    CRC32C = crc32cIntelC (crc32cInit(), buffer, BUFFER_SIZE + sizeof(uint32_t));
    CRC32C = crc32cFinish(CRC32C);
    printf("Buffer CRC32C = 0x%08x\n", CRC32C);
    
    // put the CRC at the end of the buffer
    p_crc = (uint32_t *)&buffer[BUFFER_SIZE];
    *p_crc = htole32(CRC32C);	// write in little endian
    
    // here you would be sending and receiving
    for(int i = 0; i < 10; i++) 
      printf("0x%x\n", (unsigned)buffer[BUFFER_SIZE + sizeof(uint32_t) - 10 + i]);

    // calculate the CRC of the buffer with the CRC saved and zeroed out in the buffer
    p_crc = (uint32_t *)&buffer[BUFFER_SIZE];
    R_CRC32C = le32toh(*p_crc);		// read in little endian
    *p_crc = 0;				// zero out
    
    CRC32C = crc32cIntelC (crc32cInit(), buffer, BUFFER_SIZE + sizeof(uint32_t));
    CRC32C = crc32cFinish(CRC32C);
    printf("Received CRC32C = 0x%08x, Decoded  CRC32C = 0x%08x\n", R_CRC32C ,CRC32C);
}
    