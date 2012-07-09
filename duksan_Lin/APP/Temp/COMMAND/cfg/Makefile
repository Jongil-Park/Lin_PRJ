SRCS = cfg.c
#CFLAGS = -O1
CC = arm-linux-gcc
#CC = gcc
INCDIR = ../../include
OBJS = $(SRC):.c=.o)
INC = -I$(INCDIR)
CFLAGS = -Wall $(INC)

TARGET = cfg

all: Makefile $(TARGET)

.c.o:
#	$(CC) $(INC) -c $(CFLAGS) $*.c -o $@
	$(CC) -c $*.c -o $@

clean:
	rm -rf core $(TARGET)
	

