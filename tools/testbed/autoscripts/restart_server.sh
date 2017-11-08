#!/bin/sh

cd ~/testbed
PIDS=`ps aux | grep "python utest.py server" | grep "SCREEN" | awk '{ print $2 }'`
if [ "${PIDS}" != "" ]; then
    kill ${PIDS}
fi
if [ -f /tmp/.testbed_server_34567 ]; then
    rm -rf /tmp/.testbed_server_34567
fi
screen -L server.log -dm python utest.py server
