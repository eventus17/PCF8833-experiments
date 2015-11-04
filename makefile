CC=gcc
CFLAGS=-I.
CFLAGS+=-m32
DEPS = LCDDriver.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

make: main.o LCDDriver.o 
	gcc -o LCD-LPT.out main.o LCDDriver.o -I. -m32