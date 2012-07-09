echo "make files"
cd ./config
make clean -s
make -s
cd ..
echo "copy files"
cp ./config/cfg /root/AESOP/bin


