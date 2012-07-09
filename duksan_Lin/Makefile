all : duksan util library driver command

duksan:
	cd ./APP;image.sh
driver:
	cd ./DRIVER;make;
command:
	cd ./COMMAND;make
library:
	cd ./LIBRARY;make
util:
	cd ./UTIL;make
clean :
	cd ./APP;make clean
	cd ./DRIVER;make clean
	cd ./COMMAND;make clean
	cd ./LIBRARY;make clean
	cd ./UTIL;make clean


