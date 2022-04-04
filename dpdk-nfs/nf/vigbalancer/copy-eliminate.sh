#!/bin/bash

term=$1
rej_term=$2

rm -rf $term
mkdir $term

grep "$term" *.call_path | cut -d ":" -f1 > temp_file


while read -r line
do 
	if !  grep -q "$rej_term" $line ; then 
		cp $line $term
		cp $(basename $line .call_path).tracelog $term/
	fi

done < temp_file

touch $term/stateful-error-log

rm temp_file
