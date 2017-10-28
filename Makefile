#CC=gcc
CFLAGS+= -msse4.2 -mpclmul

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: base64 unittest hs_serial

deploy:
	scp hs_serial root@edison:

hs_serial: main.o dumpbuffer.o scan_frame.o gpio.o keyboardhit.o crc32intelc.o crc32inteltable.o rt.o framing.o stats.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ base64/lib/libbase64.o -lmraa -lrt 	

unittest: unittest.o scan_frame.o crc32intelc.o crc32inteltable.o stats.o framing.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ base64/lib/libbase64.o 	
clean:
	rm -f unittest hs_serial  *.o
	$(MAKE) -C ./base64 clean
	
.PHONY: base64
base64:
	SSSE3_CFLAGS=-mssse3 $(MAKE) -C ./base64 all

