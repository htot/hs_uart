#include "hs_serial.h"

int FrameTransmitBuffer(char * TransmitBuffer, const uint32_t MessageNumber, const char * DataBuffer, const uint32_t n) {
    char tempBuffer[MAX_BUFFER];
    unsigned int pos = 0;
    uint32_t CRC32C, * p_uint32;

    p_uint32 = (uint32_t *)tempBuffer;  
    *p_uint32 = htole32(MessageNumber);         // start with message number in little endian
    *(p_uint32 + 1) = htole32(n);               // then message size in LE
    memcpy(tempBuffer + 2 * sizeof(uint32_t), DataBuffer, n); // then the data
    // put 4 zero after the data
    p_uint32 = (uint32_t *)(tempBuffer + n + 2 * sizeof(uint32_t));
    *p_uint32 = 0;

    // calculate the CRC
    CRC32C = crc32cIntelC (crc32cInit(), tempBuffer, n + 3 * sizeof(uint32_t));
    CRC32C = crc32cFinish(CRC32C);
    // put the CRC in the zeroes
    *p_uint32 = htole32(CRC32C);                // then CRC32c in little endian

    TransmitBuffer[pos++] = 0xFF;               // prefix with 2 PREAMBLES
    TransmitBuffer[pos++] = 0xFF;
    TransmitBuffer[pos++] = 0x02;               // and STX
    pos += base64_encode(&TransmitBuffer[pos], tempBuffer, n + 3 * sizeof(uint32_t));
    TransmitBuffer[pos++] = 0x03;               // postfix with ETX

    return pos;
}


// ReceiveBuffer contains all character in between STX and ETX encoded base64
int UnframeReceiveBuffer(char * DataBuffer, uint32_t * MessageNumber, const char * ReceiveBuffer, const uint32_t n) {
    char tempBuffer[MAX_BUFFER];
    uint32_t * p_uint32, Len, CRC32C, R_CRC32C;;

    if((Len = base64_decode(tempBuffer, ReceiveBuffer, n)) < 3 * sizeof(uint32_t)) return -1; // must have at least message num, size and crc

    p_uint32 = (uint32_t *)tempBuffer;
    *MessageNumber = le32toh(*p_uint32);         // from LE
    Len -= 3 * sizeof(uint32_t);                // data length without
    if(Len != le32toh(*(p_uint32 + 1))) return -1;  // we lost data from the frame
    p_uint32 = (uint32_t *)(tempBuffer + Len + 2 * sizeof(uint32_t));
    R_CRC32C = le32toh(*p_uint32);              // read CRC32C in little endian
    *p_uint32 = 0;                              // zero out
    CRC32C = crc32cIntelC (crc32cInit(), tempBuffer, Len + 3 * sizeof(uint32_t));
    CRC32C = crc32cFinish(CRC32C);
    if(CRC32C != R_CRC32C) return -1;
    memcpy(DataBuffer, tempBuffer, Len);        // then the data
    return Len;
}
