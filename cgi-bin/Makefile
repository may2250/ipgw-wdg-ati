ifeq ($(RUN_OS),)
RUN_OS := 2
endif

################################
ifeq ($(RUN_OS), 0)
#----------------------------------------------------------------------------------
CROSS_TOOL = 
LIBCGI_NAME = libcgic_pc.a
CFLAGS=-o2 -Wall -DPC_OS
TARGET_FOLD = ../cgi-bin
#----------------------------------------------------------------------------------
endif
################################
ifeq ($(RUN_OS), 2)
#----------------------------------------------------------------------------------
CROSS_TOOL = arm-linux-
LIBCGI_NAME = libcgic_arm.a
CFLAGS=-o2 -Wall -DARM_OS
TARGET_FOLD = /home/nfs/wdt-ati
CFLAGS +="-I/opt/eldk-4.2/arm/usr/include/"
LDFLAGS += "-Wl,-rpath,/opt/eldk-4.1/arm/lib/"
LDFLAGS += -L./ -lm
#----------------------------------------------------------------------------------
endif
################################

CC=$(CROSS_TOOL)gcc
AR=$(CROSS_TOOL)ar
LD=$(CROSS_TOOL)ld

LIBS=-L./ $(LIBCGI_NAME)

APP_H = -I ../../app/

all: index.cgi upload.cgi

index.cgi: cJSON.o index.o web.o udpsend.o tcpClient.o cmd.o $(LIBCGI_NAME)
	$(CC) $(CFLAGS) cJSON.o index.o web.o udpsend.o tcpClient.o cmd.o -o index.cgi ${LIBS} $(LDFLAGS)

cJSON.o: cJSON.c cJSON.h
	$(CC) $(CFLAGS)  $(APP_H) -c  cJSON.c -o cJSON.o	
	
index.o: index.c 
	$(CC) $(CFLAGS)  $(APP_H) -c  index.c -o index.o

web.o: web.c  web.h
	$(CC) $(CFLAGS)  $(APP_H) -c web.c -o web.o

cmd.o: cmd.c cmd.h 
	$(CC) $(CFLAGS)  $(APP_H) -c cmd.c -o cmd.o
	
udpsend.o: udpsend.c udpsend.h
	$(CC) $(CFLAGS)  $(APP_H) -c udpsend.c -o udpsend.o
  
  
tcpClient.o: tcpClient.c tcpClient.h 
	$(CC) $(CFLAGS)  $(APP_H) -c tcpClient.c -o tcpClient.o
	 
#upload.o: upload.c upload.h 
#	$(CC) $(CFLAGS)  $(APP_H) -c upload.c -o upload.o

upload.cgi: upload.c upload.h
	$(CC) $(CFLAGS) upload.c -o upload.cgi ${LIBS} $(LDFLAGS)

install: index.cgi upload.cgi
	cp -rf index.cgi $(TARGET_FOLD)/
	cp -rf upload.cgi $(TARGET_FOLD)/
	
clean:
	rm -f *.o index.cgi # upload.cgi

