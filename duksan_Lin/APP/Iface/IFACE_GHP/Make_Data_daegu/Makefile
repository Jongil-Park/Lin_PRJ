.SUFFIXES : .c .o 

CC = arm-linux-gcc

#INC =
#LIBS =
#CFLAGS = -g $(INC) -fPIC <- -fPIC Ãß°¡

OBJS = mkdata.o
SRCS = mkdata.c

TARGET = mkdata

all : $(TARGET)
$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS)

dep :
	gccmakedep $(INC) $(SRCS)

clean :
	rm -rf $(OBJS) $(TARGET) core
