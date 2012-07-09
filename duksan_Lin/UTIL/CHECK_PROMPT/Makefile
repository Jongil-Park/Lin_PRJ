SRCS = check_prompt.c
CC = arm-linux-gcc
INCDIR = ../../include
OBJS = $(SRC):.c=.o)
INC = -I$(INCDIR)
CFLAGS = -Wall $(INC)

TARGET = check_prompt

all: Makefile $(TARGET)

.c.o:
	$(CC) -c $*.c -o $@

clean:
	rm -rf core $(TARGET)
	

