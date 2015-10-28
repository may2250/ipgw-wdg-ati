CROSS_COMPILE=arm-linux-
STRIP	= $(CROSS_COMPILE)strip
CC	= $(CROSS_COMPILE)gcc
TARGET=arm-linux

IFLAGS = -I/opt/eldk-4.2/arm/usr/include -I/home/long/net-snmp-arm/include
CFLAGS += -O2 -Wall -DARM_OS
LDFLAGS += "-Wl,-rpath,/opt/eldk-4.1/arm/lib/"
LDFLAGS += "-L/home/long/work/snmp-build/lib/"

OBJ = alarm.o cmd.o tcpClient.o udpsend.o

.PHONY: all clean distclean

all:build install

build:
	$(CC) $(CFLAGS) $(IFLAGS) -c tcpClient.c
	$(CC) $(CFLAGS) $(IFLAGS) -c udpsend.c
	$(CC) $(CFLAGS) $(IFLAGS) -c cmd.c
	$(CC) $(CFLAGS) $(IFLAGS) -c alarm.c
	$(CC) $(OBJ) -o alarm $(LDFLAGS) -lnetsnmp -lcrypto -lz
	$(STRIP) alarm

clean:
	rm -f *.o alarm
