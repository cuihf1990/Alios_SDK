#!/user/bin/env bash

cd $(git rev-parse --show-toplevel)
cd tools/testbed/

echo -e "publish and restart server at kfcserver\n"
scp *.py kfcserver:/home/lc122798/testbed/
ssh kfcserver 'tbserver_start'
sleep 1

echo -e "\npublish and restart client at Pi1\n"
scp *.py Pi1:/home/pi/testbed/
ssh Pi1 'tbclient_start'

echo -e "\npublish and restart client at Pishanghai\n"
scp *.py Pishanghai:/home/pi/testbed/
ssh Pishanghai 'tbclient_start'

echo -e "\npublish and restart client at aoslab01\n"
scp *.py lab1:/home/iot/testbed/
ssh lab1 'tbclient_start'

echo -e "\npublish and restart client at aoslab02\n"
scp *.py lab2:/home/iot/testbed/
ssh lab2 'tbclient_start'

echo -e "\npublish and restart client at aoslab03\n"
scp *.py lab3:/home/iot/testbed/
ssh lab3 'tbclient_start'

echo -e "\nsynchronize to mac and windows\n"
scp *.py macpro:/Users/charlie/Downloads/share/testbed

echo -e "\ndone"
