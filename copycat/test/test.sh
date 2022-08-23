#!/usr/bin/bash

bufferSize=1
multiple=2

rm -rf /tmp/copycat_tmp_dummyFile /tmp/copycat_tmp_random test/results.dat

head -c 16M < /dev/urandom > /tmp/copycat_tmp_random

for i in `seq 1 18`
do
	rm -rf /tmp/copycat_tmp$bufferSize
	bufferSize=$(($bufferSize*$multiple))
done

bufferSize=1
for i in `seq 1 18`
do
	for j in `seq 1 8`
	do
		{ time ./copycat /tmp/copycat_tmp_random -o /tmp/copycat_tmp_dummyFile -b $bufferSize ; } &>> /tmp/copycat_tmp$bufferSize
		cat /tmp/copycat_tmp$bufferSize | grep real | cut -d'm' -f2 | cut -d's' -f1 | awk '{print 16 / ($0 + 0.0000001)}' | awk -v var="$i " '{print var $0}' &>> test/results.dat
	done
	bufferSize=$(($bufferSize*$multiple))
done

rm -rf /tmp/copycat_tmp_dummyFile /tmp/copycat_tmp_random

bufferSize=1
for i in `seq 1 18`
do
	rm -rf /tmp/copycat_tmp$bufferSize
	bufferSize=$(($bufferSize*$multiple))
done
