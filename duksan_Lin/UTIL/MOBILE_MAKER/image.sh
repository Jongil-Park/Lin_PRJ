echo "   CL   m_maker"
make -s clean
echo "   MK   m_maker"
make -s
cp m_maker /root/AESOP/duksan/APP
cp m_maker /root/AESOP/bin/m_maker
echo "   CP   m_maker"
