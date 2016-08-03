#CC=gcc
CFLAGS+= -msse4.2 -mpclmul

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: hs_serial filestress

hs_serial: main.o hs_serial.o dumpbuffer.o basereader.o gpio.o keyboardhit.o encoding.o crc32intelc.o crc32inteltable.o rt.o framing.o stats.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lmraa -lrt	

filestress: filestress.o 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	
clean:
	rm -f filestress hs_serial  *.o
