
echo "   MK   pset"
cd ./pset
make clean -s
make -s
cp pset /root/AESOP/bin
echo "   CP   pset"
cd ..

echo "   MK   pinfo"
cd ./pinfo
make clean -s
make -s
cp pinfo /root/AESOP/bin
echo "   CP   pinfo"
cd ..

echo "   MK   debug"
cd ./debug
make clean -s
make -s
cp debug /root/AESOP/bin
echo "   CP   debug"
cd ..

echo "   MK   mtr"
cd ./mtr
make clean -s
make -s
cp mtr /root/AESOP/bin
echo "   CP   mtr"
cd ..

echo "   MK   preq"
cd ./preq
make clean -s
make -s
cp preq /root/AESOP/bin
echo "   CP   preq"
cd ..

echo "   MK   node"
cd ./node
make clean -s
make -s
cp node /root/AESOP/bin
echo "   CP   node"
cd ..

echo "   MK   cfg"
cd ./cfg
make clean -s
make -s
cp cfg /root/AESOP/bin
echo "   CP   cfg"
cd ..

echo "   MK   view"
cd ./view
make clean -s
make -s
cp view /root/AESOP/bin/vw
echo "   CP   vw"
cp view /root/AESOP/bin/view
echo "   CP   view"
cd ..

echo "   MK   apset"
cd ./apset
make clean -s
make -s
cp apset /root/AESOP/bin/apset
echo "   CP   apset"
cd ..

echo "   MK   pdef"
cd ./pdef
make clean -s
make -s
cp pdef /root/AESOP/bin/pdef
echo "   CP   pdef"
cd ..

echo "   MK   cal"
cd ./cal
make clean -s
make -s
cp cal /root/AESOP/bin/cal
echo "   CP   cal"
cd ..

echo "   MK   status"
cd ./status
make clean -s
make -s
cp status /root/AESOP/bin/status
echo "   CP   status"
cd ..




