#!/bin/bash

PLACEgit_cont=`pwd`
if [ $# != 0 ]
then
	if [ $1 = "-h" ]
	then
		echo "AVR_mbedのgithub調整用"
		echo ". git_cont.sh (オプション)"
		echo "               change (branch名)"
		echo "               all_push"
		echo "               all_merge"
		return 0
	fi
fi
. ~/rm_~_file.sh
cd ~/AVR_mbed
DATE=`date '+%m%d_%H%M_%S'`
if [ $# = 2 ]
then
	if [ $1 = "change" ]
	then
		DATA=(`git branch --list --color`)
		FIL=(`ls`)
		for arg in ${DATA[@]}
		do
			for arg2 in ${FIL[@]}
			do
				if [ $arg = $arg2 ]
				then
					break
				fi
			done
			if [ $arg != $arg2 ]
			then
				if [ $arg ] #arg's color is green
					break
				fi
			fi
		done
		git add -A
		git commit -m $DATE
		git push AVR_mbed $arg
		git checkout $2
	fi
elif [ $# = 1 ]
then
	if [ $1 = "all_push" ]
	then
		git add -A
		git commit -m $DATE
		DATA=(`git branch --list`)
		FIL=(`ls`)
		for arg in ${DATA[@]}
		do
			for arg2 in ${FIL[@]}
			do
				if [ $arg = $arg2 ]
				then
					break
				fi
			done
			if [ $arg != $arg2 ]
			then
				git checkout $arg
				git add -A
				git commit -m $DATE
				git push AVR_mbed $arg
			fi
		done
	elif [ $1 = "all_merge" ]
	then
		git fetch AVR_mbed
		DATA=(`git branch --list`)
		FIL=(`ls`)
		for arg in ${DATA[@]}
		do
			for arg2 in ${FIL[@]}
			do
				if [ $arg = $arg2 ]
				then
					break
				fi
			done
			if [ $arg != $arg2 ]
			then
				git checkout $arg
				git merge AVR_mbed/$arg
			fi
		done
	fi
fi

#DATA=(`git branch --list --color`)
#FIL=(`ls`)
#for arg in ${DATA[@]}
#do
#	for arg2 in ${FIL[@]}
#	do
#		if [ $arg = $arg2 ]
#		then
#			break
#		fi
#	done
#	if [ $arg != $arg2 ]
#	then
		echo $arg
#	fi
#done
		
cd $PLACEgit_cont
