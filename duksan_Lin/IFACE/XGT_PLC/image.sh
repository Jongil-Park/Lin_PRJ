#make clean
#make all
#cp xgt_plc /root/AESOP/duksan/APP

arm-linux-gcc -c -Wall -I./ xgt_plc.c
arm-linux-gcc -lpthread -o xgt_plc xgt_plc.o lib_duksan.a
cp xgt_plc /root/AESOP/duksan/APP
