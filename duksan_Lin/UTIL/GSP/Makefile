SRCS = gsp.c
CC = arm-linux-gcc
#INCDIR = ../../include
INCDIR = ./../../INCLUDE/
OBJS = $(SRC):.c=.o)
INC = -I$(INCDIR)
CFLAGS = -Wall $(INC)

TARGET = gsp

all: Makefile $(TARGET)

.c.o:
	$(CC) -c $*.c -o $@

clean:
	rm -rf core $(TARGET)
	

