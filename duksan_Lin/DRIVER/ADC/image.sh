echo "   CL   dev_adc"
make -s clean
echo "   MK   dev_adc"
make -s
echo "   CP   dev_adc"
cp dev_adc.ko /root/AESOP/root


