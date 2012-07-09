SRCS = open_uart1.c
CC = arm-linux-gcc
INCDIR = ../../include
OBJS = $(SRC):.c=.o)
INC = -I$(INCDIR)
CFLAGS = -Wall $(INC)

TARGET = open_uart1

all: Makefile $(TARGET);cp $(TARGET) open_net32

.c.o:
	$(CC) -c $*.c -o $@

clean:
	rm -rf core $(TARGET)
	

