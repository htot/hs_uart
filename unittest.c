#include "hs_serial.h"

int DebugFlag = 0;
long msecs = 15;


/**
 * @brief ...
 * 
 * @param argc p_argc:...
 * @param argv p_argv:...
 * @return int
 */
int main(int argc, char** argv)
{
    unsigned char writebuffer[2 * MAX_BUFFER];
    unsigned char textbuffer[DATA_BUFFER];
    unsigned char readbuffer[3072];
    unsigned char error[] = {0x00, 0xFF, 0x01};
    size_t TransmitSize, BufferSize = 1024, i, OutLen;
    int j;
    
    //fill textbuffer with random
    for(i = 0; i < DATA_BUFFER; i++)
    {
        textbuffer[i] = (char)i;
    };
    
    // Test 1: Send Single Message and Decode
    uint32_t WriteMessageNumber = 10, ReadMessageNumber = 0;
    TransmitSize = FrameTransmitBuffer(writebuffer, WriteMessageNumber, textbuffer, BufferSize);
    if(Scan_Frame(writebuffer, TransmitSize, readbuffer, &OutLen, &ReadMessageNumber) < 0) {
        printf("Test 1: Send Single Message and Decode: nothing received\n");
        exit(EXIT_FAILURE);
    };
    if(WriteMessageNumber != ReadMessageNumber) {
        printf("Test 1: Send Single Message and Decode: WriteMessageNumber != ReadMessageNumber\n");
        exit(EXIT_FAILURE);
    };
    if(OutLen != BufferSize) {
        printf("Test 1: Send Single Message and Decode: OutLen != BufferSize\n");
        exit(EXIT_FAILURE);
    };
    if(memcmp(textbuffer, readbuffer, BufferSize) != 0) {
        printf("Test 1: Send Single Message and Decode: Data does not match\n");
        exit(EXIT_FAILURE);
    };
    if(Scan_Frame(writebuffer, 0, readbuffer, &OutLen, &ReadMessageNumber) >= 0) {
        printf("Test 1: Send Single Message and Decode: buffer not empty\n");
        exit(EXIT_FAILURE);
    };

    // Test 2: Send two Messages and Decode
    TransmitSize = FrameTransmitBuffer(writebuffer, WriteMessageNumber, textbuffer, BufferSize);
    TransmitSize = FrameTransmitBuffer(writebuffer + TransmitSize, WriteMessageNumber + 1, textbuffer, BufferSize);
    if(Scan_Frame(writebuffer, 2 * TransmitSize, readbuffer, &OutLen, &ReadMessageNumber) < 0) {
        printf("Test 2: Send two Messages and Decode: nothing received\n");
        exit(EXIT_FAILURE);
    };
    if(WriteMessageNumber != ReadMessageNumber) {
        printf("Test 2: Send two Messages and Decode: WriteMessageNumber != ReadMessageNumber\n");
        exit(EXIT_FAILURE);
    };
    if(OutLen != BufferSize) {
        printf("Test 2: Send two Messages and Decode: OutLen != BufferSize\n");
        exit(EXIT_FAILURE);
    };
    if(Scan_Frame(writebuffer, 0, readbuffer, &OutLen, &ReadMessageNumber) < 0) {
        printf("Test 2: Send two Messages and Decode: 2nd message not received\n");
        exit(EXIT_FAILURE);
    };
    if(WriteMessageNumber + 1 != ReadMessageNumber) {
        printf("Test 2: Send two Messages and Decode: 2nd WriteMessageNumber != ReadMessageNumber\n");
        exit(EXIT_FAILURE);
    };
    if(OutLen != BufferSize) {
        printf("Test 2: Send two Messages and Decode: OutLen != BufferSize\n");
        exit(EXIT_FAILURE);
    };

    // Test 3: Send message in 2 parts
    TransmitSize = FrameTransmitBuffer(writebuffer, WriteMessageNumber, textbuffer, BufferSize);
    for(i = 0; i < TransmitSize; i++) {
        if(Scan_Frame(writebuffer, i, readbuffer, &OutLen, &ReadMessageNumber) >= 0) {
            printf("Test 3: Send message in 2 parts: Should not decode first part\n");
            exit(EXIT_FAILURE);
        };
        if(Scan_Frame(writebuffer + i, TransmitSize - i, readbuffer, &OutLen, &ReadMessageNumber) < 0) {
            printf("Test 3: Send message in 2 parts: 2nd part not decoded\n");
            exit(EXIT_FAILURE);
        };
        if(WriteMessageNumber != ReadMessageNumber) {
            printf("Test 3: Send message in 2 parts: WriteMessageNumber != ReadMessageNumber\n");
            exit(EXIT_FAILURE);
        };
        if(OutLen != BufferSize) {
            printf("Test 3: Send message in 2 parts: OutLen != BufferSize\n");
            exit(EXIT_FAILURE);
        };
    };
    // Test 4: Send 2 messages, but overwrite 1 char in first
    for(j = 0; j < sizeof(error); j++) {
        for(i = 0; i < TransmitSize; i++) {
            TransmitSize = FrameTransmitBuffer(writebuffer, WriteMessageNumber, textbuffer, BufferSize);
            TransmitSize = FrameTransmitBuffer(writebuffer + TransmitSize, WriteMessageNumber + 1, textbuffer, BufferSize);
            writebuffer[i] = error[j];
            if(Scan_Frame(writebuffer, 2 * TransmitSize, readbuffer, &OutLen, &ReadMessageNumber) < 0) {
                printf("Test 4: Send 2 messages, but overwrite 1 char in first: nothing received\n");
                exit(EXIT_FAILURE);
            };
            // in some cases the first message doesn't break, that is normal
            if((i == 0) ||  // any char may break the first PREAMBLE
            ((i == 1) && (j == 1)) || // writing a PREAMBLE on top of PREAMBLE will not break the message
            (((i == 1385) || (i == 1386)) && (j == 2))) {
                if(WriteMessageNumber != ReadMessageNumber) {
                    printf("Test 4: Send 2 messages, but overwrite 1 char in first: First not received\n");
                    exit(EXIT_FAILURE);
                };
                if(Scan_Frame(writebuffer, 0, readbuffer, &OutLen, &ReadMessageNumber) < 0) {
                    printf("Test 4: Send 2 messages, but overwrite 1 char in first: 2nd not received\n");
                    exit(EXIT_FAILURE);
                };
            };
            if(WriteMessageNumber + 1 != ReadMessageNumber) {
                printf("Test 4: Send 2 messages, but overwrite 1 char in first: 2nd message not received\n");
                exit(EXIT_FAILURE);
            };
            if(OutLen != BufferSize) {
                printf("Test 4: Send 2 messages, but overwrite 1 char in first: OutLen != BufferSize\n");
                exit(EXIT_FAILURE);
            };
            if(Scan_Frame(writebuffer, 0, readbuffer, &OutLen, &ReadMessageNumber) >= 0) {
                printf("Test 4: Send 2 messages, but overwrite 1 char in first: buffer not empty\n");
                exit(EXIT_FAILURE);
            };

        };
    };

    printf("All tests passed!");
}
