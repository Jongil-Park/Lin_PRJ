all : 
	cd ./AUTOUPLOAD;make
	cd ./AUTOUPLOAD;cp upload /root/AESOP/duksan/APP/upload
	cd ./AUTOUPLOAD;cp upload /root/AESOP/duksan_rootfs/duksan/APP/upload
	cd ./FILE_COPY;make
	cd ./FILE_COPY;cp fcopy /root/AESOP/duksan/APP/fcopy
	cd ./FILE_COPY;cp fcopy /root/AESOP/duksan_rootfs/duksan/APP/fcopy
	cd ./GSP;make
	cd ./GSP;cp gsp /root/AESOP/duksan/APP/gsp
	cd ./GSP;cp gsp /root/AESOP/duksan_rootfs/duksan/APP/gsp
	cd ./HTTPD/mini_httpd-1.19;make
	cd ./HTTPD/mini_httpd-1.19;cp mini_httpd /root/AESOP/httpd/mini_httpd
	cd ./HTTPD/mini_httpd-1.19;cp mini_httpd /root/AESOP/duksan_rootfs/httpd/mini_httpd
	cd ./MOBILE_MAKER;make
	cd ./MOBILE_MAKER;cp m_maker /root/AESOP/duksan/APP/m_maker
	cd ./MOBILE_MAKER;cp m_maker /root/AESOP/duksan_rootfs/duksan/APP/m_maker
	cd ./OPEN_UART1;make
	cd ./OPEN_UART1;cp open_net32 /root/AESOP/duksan/APP/open_net32
	cd ./OPEN_UART1;cp open_net32 /root/AESOP/duksan_rootfs/duksan/APP/open_net32
	cd ./CHECK_PROMPT;./image.sh
	cd ./CHECK_PROMPT;cp check_prompt /root/AESOP/duksan/APP/check_prompt
	cd ./CHECK_PROMPT;cp check_prompt /root/AESOP/duksan_rootfs/duksan/APP/check_prompt
clean :
	cd ./AUTOUPLOAD;make clean
	cd ./FILE_COPY;make clean
	cd ./GSP;make clean
	cd ./HTTPD/mini_httpd-1.19;make clean
	cd ./MOBILE_MAKER;make clean
	cd ./OPEN_UART1;make clean
	cd ./CHECK_PROMPT;make clean


