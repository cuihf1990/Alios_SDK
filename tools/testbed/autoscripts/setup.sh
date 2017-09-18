#!/bin/sh

sudo apt-get update
sudo apt-get -y upgrade

sudo apt-get -y install vim mutt msmtp
sudo apt-get -y install python-pip git screen
sudo pip install pyserial

cwd=`pwd`
if [ ! -d ~/tools ]; then
    mkdir ~/tools
fi
cd ~/tools
git clone https://github.com/espressif/esptool.git
echo LC_ALL=\"en_US.UTF-8\" >> ~/.bashrc
echo "export ESPTOOL_PATH=~/tools/esptool" >> ~/.bashrc

cd $cwd
sudo cp msmtprc /root/.msmtprc
sudo chown root:root /root/.msmtprc
sudo chmod 600 /root/.msmtprc
sudo cp muttrc /root/.muttrc
sudo chown root:root /root/.muttrc
sudo chmod 600 /root/.muttrc
sudo cp send-ip-mail.sh /root/
sudo chown root:root /root/send-ip-mail.sh
sudo cp rc.local /etc/rc.local
sudo chown root:root /etc/rc.local
sudo cp 98-usb-serial.rules /etc/udev/rules.d/


