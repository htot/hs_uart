#CC=gcc
#CFLAGS=-I.

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: c_serial_test mraa_spi_test mraa-gpio-example cycle-pwm3 iobus-iface

c_serial_test: c_serial_test.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lmraa

iobus-iface: iobus-iface.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lmraa

mraa_spi_test: mraa_spi_test.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lmraa

mraa-gpio-example: mraa-gpio-example.c 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lmraa

cycle-pwm3: cycle-pwm3.c 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^  -lmraa
	
yuna_test1: yuna_test1.c 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lmraa

yuna_test2: yuna_test2.c 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lmraa

yuna_readdelay: yuna_readdelay.c 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lmraa

yuna_base64: yuna_base64.c encoding.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lmraa
	
feyuna_base64: feyuna_base64.c encoding.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lmraa -lrt

hs_serial: main.o hs_serial.o dumpbuffer.o basereader.o gpio.o keyboardhit.o encoding.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lmraa -lrt	

