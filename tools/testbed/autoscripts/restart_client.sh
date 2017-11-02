#!/bin/sh

cd ~/testbed
PIDS=`ps aux | grep "python utest.py client" | grep "SCREEN" | awk '{ print $2 }'`
if [ "${PIDS}" != "" ]; then
    kill ${PIDS}
fi
if [ -f /tmp/.testbed_client ]; then
    rm -rf /tmp/.testbed_client
fi
screen -dmL python utest.py client -s 10.125.52.132
