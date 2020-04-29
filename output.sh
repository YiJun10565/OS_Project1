#! /usr/bin/env bash

extension="_stdout.txt"
extension2="_dmesg.txt"
extension3="_finish.txt"
context=$(ls ./OS_PJ1_Test)
for i in $context
do
	#remove the .txt
	prefix=$(echo $i | sed "s/....$//g")
	echo $prefix
	dmesg -C
	./a.out < "./OS_PJ1_Test/$i" > ./output/"$prefix$extension" 2> ./output/"$prefix$extension3"
	sleep 1
	dmesg | grep Project1 > ./output/"$prefix$extension2"
	
done
