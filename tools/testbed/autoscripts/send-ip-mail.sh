#!/bin/bash

# check network availability
while true
do
    TIMEOUT=5
    SITE_TO_CHECK="www.126.com"
    RET_CODE=`curl -I -s --connect-timeout $TIMEOUT $SITE_TO_CHECK -w %{http_code} | tail -n1`
    if [ "x$RET_CODE" = "x200" ]; then
    echo "Network OK, will send mail..."
    break
    else
    echo "Network not ready, wait..."
    sleep 1s
    fi
done

# get the IP address of eth0, e.g. "192.168.16.5"
ETH0_IP_ADDR=`ifconfig eth0 | sed -n "2,2p" | awk '{print substr($2,1)}'`
WLAN0_IP_ADDR=`ifconfig wlan0 | sed -n "2,2p" | awk '{print substr($2,1)}'`

# send the Email
echo "eth-$ETH0_IP_ADDR wifi-$WLAN0_IP_ADDR Current time: `date '+%F %T'`." | mutt -s "Raspberry Pi IPs: eth-$ETH0_IP_ADDR wifi-$WLAN0_IP_ADDR" lc122798@alibaba-inc.com
