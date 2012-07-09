echo "GPIO Module Clean"
cd ./gpio
make -s  clean
cd ..
echo "LED Module Clean"
cd ./led
make -s clean
cd ..
echo "Prompt Clean"
cd ./prompt
make -s clean
cd ..
