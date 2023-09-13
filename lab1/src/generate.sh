#!/bin/bash

for ((i=0; i < 150; i++))
do
echo $(od -A n -t d -N 1 /dev/urandom) >> numbers.txt
done