#echo "GPIO Module"
#cd ./gpio
#make -s
#cd ..

cd ./adc
make clean -s
echo "   CL   dev_adc"
make clean -s
echo "   MK   dev_adc"
make -s
echo "   CP   dev_adc"
cp dev_adc.ko /root/AESOP/duksan/MODULE/
cd ..


#echo "LED Module"
#cd ./led
#make -s
#cd ..
#echo "Prompt Util"
#cd ./prompt
#make -s
#cd ..

#cp ./gpio/dev_gpio.ko /root/AESOP/duksan/MODULE
#echo "copy module header file"
#cp ./gpio/dev_gpio.h ./../include/
#cp ./led/dev_led.h ./../include/
