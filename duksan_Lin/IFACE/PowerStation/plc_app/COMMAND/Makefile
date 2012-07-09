OUTPUT = cmd
OBJECTS = cmd.o
INC = -I./../INCLUDE
CFLAGS = -O1
CC = arm-linux-gcc

$(OUTPUT) : $(OBJECTS)
	$(CC) -o $(OUTPUT) $(OBJECTS) $(INC) $(CFLAGS)

cmd.o : cmd.c
	$(CC) -o cmd.o -c cmd.c $(INC) $(CFLAGS)

clean:
	rm -rf $(OUTPUT) cmd.o

