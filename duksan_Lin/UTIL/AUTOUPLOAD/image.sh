echo "   CL   upload"
make -s clean
echo "   MK   upload"
make -s
cp upload /root/AESOP/duksan/APP
echo "   CP   upload"
