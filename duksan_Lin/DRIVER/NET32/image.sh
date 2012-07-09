echo "   CL   net32_dev"
make -s clean
echo "   MK   net32_dev"
make -s
echo "   CP   net32_dev"
cp net32_dev.ko /root/AESOP/duksan/MODULE/net32_dev.ko


