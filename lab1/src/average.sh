#!/bin/bash

for var in "$@"; do
    sum=$(( $sum + $var ))
    count=$(( $count + 1 ))
done
average=$(( $sum / $count ))
echo "Среднее арифметическое = $average"
echo "Количество = $count"