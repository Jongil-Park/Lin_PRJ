arm-linux-gcc -c -Wall -I./ test.c
arm-linux-gcc -lpthread -o test test.o lib_duksan.a
cp test /root/AESOP/duksan/APP/test





