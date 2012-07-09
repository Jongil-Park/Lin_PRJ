cp RUN_bjs /root/AESOP/lsu/
cp .configPLC /root/AESOP/lsu/
cp configEth0 /root/AESOP/lsu/
cp COMMAND/cmd /root/AESOP/lsu/

cp RUN_bjs gcu_main/
cp RUN_bjs gcu_backup/
cp cmd gcu_main/
cp cmd gcu_backup/


tar cvf ../BJS_Tar.tar ../plc_app/

