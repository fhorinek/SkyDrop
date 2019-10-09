#!/bin/sh

for f in $( ls source );
do
    nohup ./convert.py -l source/$f &
done

while (( $(ps aux | pgrep -f -a convert.py | wc -l) != 0))
do 
    ps aux | grep convert.py
    sleep 5
    echo "##################################################################"
done