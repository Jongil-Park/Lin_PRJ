echo "   CL   httpd" 
make -s clean
echo "   MK   httpd" 
make -s
echo "   CP   httpd"
cp mini_httpd /root/AESOP/httpd

