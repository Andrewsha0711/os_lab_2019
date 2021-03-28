#!/bin/bash
source_file=$1
sum=0
amount=0
while read LINE; do 
    ((sum+=$LINE))
    ((amount++))
done < $source_file
echo "$amount"
echo "$(($sum/$amount))"