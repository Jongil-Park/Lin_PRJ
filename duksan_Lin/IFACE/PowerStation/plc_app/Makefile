OUTPUT = RUN_bjs
OBJECTS = main.o PLC_OBSERVER/plc_observer.o DNP_OBSERVER/dnp_observer.o COMMAND/cmd_server.o

CC = arm-linux-gcc
OPT = -O
INC = -I./INCLUDE
THR = -lpthread
CFLAGS = -O1

$(OUTPUT) : $(OBJECTS)
		$(CC) -o $(OUTPUT) $(OBJECTS) $(INC) $(THR) $(CFLAGS)

main.o : main.c
		$(CC) -o main.o -c main.c $(INC) $(CFLAGS) 

PLC_OBSERVER/plc_observer.o : PLC_OBSERVER/plc_observer.c
		$(CC) -o PLC_OBSERVER/plc_observer.o -c PLC_OBSERVER/plc_observer.c $(INC) $(CFLAGS)


DNP_OBSERVER/dnp_observer.o : DNP_OBSERVER/dnp_observer.c
		$(CC) -o DNP_OBSERVER/dnp_observer.o -c DNP_OBSERVER/dnp_observer.c $(INC) $(CFLAGS)

COMMAND/cmd_server.o : COMMAND/cmd_server.c
		$(CC) -o COMMAND/cmd_server.o -c COMMAND/cmd_server.c $(INC) $(CFLAGS)

all:
	make
	cd COMMAND;make
	cd ..
	cp COMMAND/cmd cmd
	cp RUN_bjs /root/AESOP/lsu/
	cp .configPLC /root/AESOP/lsu/
	cp configEth0 /root/AESOP/lsu/
	cp COMMAND/cmd /root/AESOP/lsu/
	cp RUN_bjs gcu_main/
	cp RUN_bjs gcu_backup/
	cp cmd gcu_main/
	cp cmd gcu_backup/
	tar cvf ../BJS_Tar.tar ../plc_app/

clean:
	rm -f $(OUTPUT) main.o PLC_OBSERVER/plc_observer.o DNP_OBSERVER/dnp_observer.o COMMAND/cmd_server.o
	cd COMMAND;make clean
	cd ..
	rm cmd

