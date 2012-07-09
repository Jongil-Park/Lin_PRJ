all : xgt_plc

xgt_plc : xgt_plc.c
	arm-linux-gcc -o $@ $< -lpthread

clean :
	rm xgt_plc
