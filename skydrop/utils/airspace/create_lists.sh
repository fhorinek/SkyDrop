#!/bin/sh

for f in $( ls source );
do
    nohup ./convert.py -l source/$f &
done

watch "ps aux | grep convert.py"

