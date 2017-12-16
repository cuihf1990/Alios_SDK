#!/bin/bash

targets="server lab1 lab2 lab3 Pi1 Pi2 Pi1-SH Pi2-SH"
server=tc1
cd $(git rev-parse --show-toplevel)
cd tools/testbed/

if [ $# -gt 0 ]; then
    targets=$@
fi

for target in $@; do
    if [ "${target}" = "server" ]; then
        echo -e "publish and restart server\n"
        scp *.py ${server}:/home/lc122798/testbed/
        scp -r board ${server}:/home/lc122798/testbed/
        ssh ${server} 'tbserver_start'
        sleep 2
    elif [[ ${target} == lab* ]]; then
        echo -e "\npublish and restart client at aos${target}\n"
        scp *.py ${target}:/home/iot/testbed/
        scp -r board ${target}:/home/iot/testbed/
        ssh ${target} 'tbclient_start'
        sleep 2
    elif [[ ${target} == Pi* ]]; then
        echo -e "\npublish and restart client at ${target}\n"
        scp *.py ${target}:/home/pi/testbed/
        scp -r board ${target}:/home/pi/testbed/
        ssh ${target} 'tbclient_start'
        sleep 2
    else
        echo -e "\nerror: unkonw publish target ${target}\n"
    fi
done
echo -e "\ndone"
