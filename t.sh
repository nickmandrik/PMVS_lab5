#!/bin/bash
function test {
	echo "$1" > /proc/first
	echo "$2" > /proc/operand
	echo "$3" > /proc/second
	res=`sudo cat /dev/result`
	echo "$1$2$3=${res}"
	if [ "${res}" == "$4" ]; then
		echo "OK"
	else 
		echo "NOT CORRECT"
	fi
}

test 5 + 13 18
test 13 + 2 15
test -4 + 10 6
test 10 / 2 5
test 10 / -2 -5
test 8 / 2 4
test 10 '*' 3 30
test 10 '*' -3 -30
test -7 '*' -7 49
test 15 - 20 -5
test 10 - -27 37
test 10 - 60 -50
test 10 - 10 0
