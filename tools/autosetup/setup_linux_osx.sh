#!/bin/bash

OS=`uname -s`

if [ "$OS" != "Darwin" ] && [ "$OS" != "Linux" ]; then
    echo "error: unsupported OS $OS"
    exit 1
fi

echo "Input which directory you want to put code (default:~/code/aos):"
read OBJ_DIR
if [ "${OBJ_DIR}" = "" ]; then
    OBJ_DIR=~/code/aos
fi

echo "Object directory is ${OBJ_DIR}"
if [ -d ${OBJ_DIR} ];then
    echo "Object directory already exist，overwrite(Y/N)?:"
    read -n 1 OVERWRITE
    if [ "${OVERWRITE}" != "Y" ] && [ "${OVERWRITE}" != "y" ];then
        exit 0
    fi
    rm -rf ${OBJ_DIR}
else
    mkdir -p ${OBJ_DIR}
    if [ $? -ne 0 ]; then
        echo "error: create directory ${OBJ_DIR} failed"
        exit 1
    fi
fi

if [ "$OS" = "Darwin" ]; then
    if [ "`which brew`" = "" ]; then
        /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
    fi
    if [ "`which pip`" = "" ];then
        sudo easy_install pip
    fi
    if [ "`which gcc`" = "" ];then
        brew install gcc
    fi
    GCC_ARM_URL="https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q3-update/+download/gcc-arm-none-eabi-5_4-2016q3-20160926-mac.tar.bz2"
    GCC_XTENSA_URL="https://dl.espressif.com/dl/xtensa-esp32-elf-osx-1.22.0-75-gbaf03c2-5.2.0.tar.gz"
    HOST_OS=OSX
else #Some Linux version
    GCC_ARM_URL="https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q3-update/+download/gcc-arm-none-eabi-5_4-2016q3-20160926-linux.tar.bz2"
    if [ "`uname -m`" = "x86_64" ]; then
        GCC_XTENSA_URL="https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-75-gbaf03c2-5.2.0.tar.gz"
        HOST_OS=Linux64
        if [ "`which apt-get`" != "" ]; then
            sudo apt-get -y install git wget make flex bison gperf tar
            sudo apt-get -y install gcc-multilib
            sudo apt-get -y install libssl-dev libssl-dev:i386
            sudo apt-get -y install libncurses-dev libncurses-dev:i386
            sudo apt-get -y install libreadline-dev libreadline-dev:i386
            sudo apt-get -y install python python-pip
        elif [ "`which yum`" != "" ]; then
            sudo yum -y install git wget make flex bison gperf tar python python-pip
            sudo yum -y install gcc gcc-c++ glibc-devel glibc-devel.i686 libgcc libgcc.i686
            sudo yum -y install libstdc++-devel libstdc++-devel.i686
            sudo yum -y install openssl-devel openssl-devel.i686
            sudo yum -y install ncurses-devel ncurses-devel.i686
            sudo yum -y install readline-devel readline-devel.i686
            sudo yum -y install python python-pip
        elif [ "`which pacman`" != "" ]; then
            sudo pacman -S --needed gcc git make ncurses flex bison gperf
        else
            echo "error: unknown package manerger"
            exit 1
        fi
    else
        GCC_XTENSA_URL="https://dl.espressif.com/dl/xtensa-esp32-elf-linux32-1.22.0-75-gbaf03c2-5.2.0.tar.gz"
        HOST_OS=Linux32
        if [ "`which apt-get`" != "" ]; then
            sudo apt-get -y install git wget make flex bison gperf tar
            sudo apt-get -y install libssl-dev libncurses-dev libreadline-dev
            sudo apt-get -y install python python-pip
        elif [ "`which yum`" != "" ]; then
            sudo yum -y install git wget make flex bison gperf tar python python-pip
            sudo yum -y install gcc gcc-c++ glibc-devel libgcc libstdc++-devel
            sudo yum -y install openssl-devel ncurses-devel readline-devel
        elif [ "`which pacman`" != "" ]; then
            sudo pacman -S --needed gcc git make ncurses flex bison gperf python2 python2-pip
        else
            echo "error: unknown package manerger"
            exit 1
        fi
    fi
fi

sudo pip install pyserial aos-cube

cd ${OBJ_DIR}/../
rm -rf ${OBJ_DIR}
echo "cloning code..."
git clone https://github.com/alibaba/AliOS-Things ${OBJ_DIR}
if [ $? -ne 0 ]; then
    echo "error: clone code failed"
    exit 1
fi
echo "cloning code...done"

echo "installing arm-gcc toolchain ..."
ARM_TOOLCHAIN_VERSION=5_4-2016q3-20160926
ARM_TOOLCHAIN_PATH=${OBJ_DIR}/build/compiler/arm-none-eabi-${ARM_TOOLCHAIN_VERSION}
if [ ! -d ${ARM_TOOLCHAIN_PATH} ]; then
    mkdir -p ${ARM_TOOLCHAIN_PATH}
fi
cd ${ARM_TOOLCHAIN_PATH}
wget ${GCC_ARM_URL}
if [ $? -ne 0 ]; then
    echo "error: download arm-none-eabi-gcc toolchain faield"
    exit 1
fi
tar jxf gcc-arm-none-eabi*.tar.bz2
rm -f gcc-arm-none-eabi*.tar.bz2
mv gcc_arm_none_eabi* ${HOST_OS}
echo "installing arm-none-eabi-gcc toolchain ...done"


echo "installing xtensa-gcc toolchain ..."
XTENSA_TOOLCHAIN_PATH=${OBJ_DIR}/build/compiler/gcc-xtensa-esp32
if [ ! -d ${XTENSA_TOOLCHAIN_PATH} ]; then
    mkdir -p ${XTENSA_TOOLCHAIN_PATH}
fi
cd ${XTENSA_TOOLCHAIN_PATH}
wget ${GCC_XTENSA_URL}
if [ $? -ne 0 ]; then
    echo "error: download xtensa-gcc toolchain faield"
    exit 1
fi
tar zxf xtensa-esp32-elf*.tar.gz
rm -f xtensa-esp32-elf*.tar.gz
mv xtensa-esp32-elf* ${HOST_OS}
echo "installing xtensa-gcc toolchain ...done"

echo "installing OpenOCD ..."
OPENOCD_PATH=${OBJ_DIR}/build/OpenOCD
OPENOCD_URL="https://files.alicdn.com/tpsservice/27ba2d597a43abfca94de351dae65dff.zip"
if [ -d ${OPENOCD_PATH} ]; then
    rm -rf ${OPENOCD_PATH}
fi
cd ${OBJ_DIR}/build/
wget ${OPENOCD_URL}
if [ $? -ne 0 ]; then
    echo "error: download OpenOCD faield"
    exit 1
fi
unzip 27ba2d*.zip
rm -f 27ba2d*.zip
rm -rf __MACOSX
echo "installing OpenOCD ...done"

echo "finieshed"

