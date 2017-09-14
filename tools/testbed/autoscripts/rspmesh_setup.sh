#!/bin/sh

sudo apt-get update
sudo apt-get -y upgrade
sudo apt-get -y install raspberrypi-kernel-headers git libgmp3-dev gawk qpdf bison flex make bc

if [ ! -d ~/tools ]; then
    mkdir -p ~/tools
fi
cd ~/tools

if [ -d nexmon ]; then
    rm -rf nexmon
fi
git clone https://github.com/seemoo-lab/nexmon.git
cd nexmon
nexmondir=`pwd`

if [ ! -f /usr/lib/arm-linux-gnueabihf/libisl.so.10 ]; then
    cd buildtools/isl-0.10
    ./configure
    make -j4
    sudo make install
    sudo ln -s /usr/local/lib/libisl.so /usr/lib/arm-linux-gnueabihf/libisl.so.10
fi

cd ${nexmondir}
source setup_env.sh
make -j4
if [ $? -ne 0 ]; then
    echo "error: build nexmon failed"
    exit 1
fi

cd patches/bcm43430a1/7_45_41_46/nexmon/
make -j4
if [ $? -ne 0 ]; then
    echo "error: build brcmfmac firmware failed"
    exit 1
fi
make backup-firmware
sudo make install-firmware

brcmpath=`modinfo brcmfmac | grep filename | awk '{print $2}'`
if [ ! -f ${brcmpath}.orig ]; then
    sudo cp ${brcmpath} ${brcmpath}.orig
fi
sudo cp ${nexmondir}/patches/bcm43430a1/7_45_41_46/nexmon/brcmfmac_kernel49/brcmfmac.ko ${brcmpath}
sudo depmod -a

cd ${nexmondir}/utilities/nexutil/
make
if [ $? -ne 0 ]; then
    echo "error: build utility failed"
    exit 1
fi
sudo make install

#optinal step
#sudo apt-get remove wpasupplicant

