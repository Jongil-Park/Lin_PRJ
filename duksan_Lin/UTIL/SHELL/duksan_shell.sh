#!/bin/sh
count=0
while [ true ]
do
#ps -ef|grep duksan > temp.txt

#check duksan appliaction status
result=$(ps -ef | grep duksan | wc -l)
#echo "result $result"
if [ $result -gt 5 ]
then
	#echo "Right"
	count=0
else
	#echo "Wrong"
	count=$(($count+1))
	#echo "$count"
fi

#run duksan application
if [ $count -gt 3 ]
then
	echo "Run Duksan Application"
	./stop
	sleep 1
	./duksan &
	count=0
fi

#wait sleep 1
sleep 1
done
