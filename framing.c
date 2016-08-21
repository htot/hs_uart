#include "hs_serial.h"

extern int DebugFlag;

// TransmitBuffer contains the whole frame, PREAMBLES, STX, encoded (MSG#, Size, Data, CRC), ETC
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
    uint32_t * p_uint32, Len, CRC32C, R_CRC32C, SizeField;

    // First we decode the ReceiveBuffer
    if((Len = base64_decode(tempBuffer, ReceiveBuffer, n)) < 3 * sizeof(uint32_t)) {
        if (DebugFlag) printf("The decoded message contains to few byte to hold message num, size and crc\n");
        return -1; // must have at least message num, size and crc, discard the whole frame
    };

    // Then we check the decoded size against the message size field
    p_uint32 = (uint32_t *)tempBuffer;
    *MessageNumber = le32toh(*p_uint32);        // from little endian (LE)
    Len -= 3 * sizeof(uint32_t);                // data length not including message num, size and crc
    if(Len != (SizeField = le32toh(*(p_uint32 + 1)))) {
        if (DebugFlag) printf("The data in the decoded message contains %i bytes, the message size field says %i\n", Len, SizeField);
        TimeEvent(OVERRUNS);
        return -1;  // we lost data from the frame, so discard the whole frame
    };

    // Finally we check the CRC32C
    p_uint32 = (uint32_t *)(tempBuffer + Len + 2 * sizeof(uint32_t));
    R_CRC32C = le32toh(*p_uint32);              // read CRC32C in LE
    *p_uint32 = 0;                              // zero out
    CRC32C = crc32cIntelC (crc32cInit(), tempBuffer, Len + 3 * sizeof(uint32_t));
    CRC32C = crc32cFinish(CRC32C);
    if(CRC32C != R_CRC32C) {
        if (DebugFlag) printf("The CRC in the decoded message does not match the calculted CRC\n");
        return -1;  // There is a CRC mismtach, so discard the whole frame
    };

    // Everthing matches, so copy the data out
    memcpy(DataBuffer, tempBuffer, Len);
    return Len;
}
