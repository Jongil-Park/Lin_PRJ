OBJECTS = cmd_client.o
SRCS = cmd_client.c
CFLAGS = -O1
CC = arm-linux-gcc
INC = -I../include

TARGET = cmd

$(TARGET) : $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(CFLAGS)

cmd_client.o : cmd_client.c
	$(CC) $(INC) -o cmd_client.o -c cmd_client.c $(CFLAGS)

clean:
	rm -rf $(TARGET) cmd_client.o

