SRCS = rpset.c
CC = arm-linux-gcc
INCDIR = ../../include
OBJS = $(SRC):.c=.o)
INC = -I$(INCDIR)
CFLAGS = -Wall $(INC)

TARGET = rpset 

all: Makefile $(TARGET)

.c.o:
	$(CC) $(INC) -c $(CFLAGS) $*.c -o $@

clean:
	rm -rf core $(TARGET)
	

