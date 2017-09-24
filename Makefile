#CC=gcc
CFLAGS+= -msse4.2 -mpclmul

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: base64 hs_serial

hs_serial: main.o hs_serial.o dumpbuffer.o basereader.o gpio.o keyboardhit.o crc32intelc.o crc32inteltable.o rt.o framing.o stats.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ base64/lib/libbase64.o -lmraa -lrt 	

unittest: unittest.o basereader.o crc32intelc.o crc32inteltable.o stats.o framing.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ base64/lib/libbase64.o 	
clean:
	rm -f filestress hs_serial  *.o
	$(MAKE) -C ./base64 clean
	
.PHONY: base64
base64:
	SSSE3_CFLAGS=-mssse3 $(MAKE) -C ./base64 all

