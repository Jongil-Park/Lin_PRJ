CC = arm-linux-gcc

SRCS = debug.c
OBJS = $(SRC):.c=.o)

INCDIR = ../../include
INC = -I$(INCDIR)
CFLAGS = -Wall $(INC)

TARGET = debug

all: Makefile $(TARGET)

.c.o:
	$(CC) $(INC) -c $(CFLAGS) $*.c -o $@

clean:
	rm -rf core $(TARGET)
	

