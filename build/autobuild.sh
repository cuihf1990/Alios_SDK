#!/usr/bin/env bash

workdir=autobuild
linux_posix_targets="alinkapp meshapp networkapp"
linux_targets="alinkapp networkapp helloworld linuxapp meshapp tls yts"
linux_platforms="linuxhost linuxhost@debug linuxhost@release"
mk3060_targets="alinkapp helloworld linuxapp meshapp tls uDataapp networkapp"
mk3060_platforms="mk3060 mk3060@release"
b_l475e_targets="mqttapp helloworld tls uDataapp networkapp"
b_l475e_platforms="b_l475e"
lpcxpresso54102_targets="helloworld alinkapp mqttapp tls networkapp"
lpcxpresso54102_platforms="lpcxpresso54102"
esp32_targets="alinkapp helloworld bluetooth.bleadv bluetooth.bleperipheral networkapp"
esp32_platforms="esp32devkitc"
esp8266_targets="helloworld"
esp8266_platforms="esp8266"
bins_type="app framework kernel"
mk3239_targets="bluetooth.ble_advertisements bluetooth.ble_show_system_time"
mk3239_platforms="mk3239"
scons_build_targets="helloworld@b_l475e helloworld@mk3060"
scons_ide_targets=""
ide_types="keil iar"

DEBUG="no"
if [ $# -gt 0 ]; then
    DEBUG=$1
fi

git status > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "error: not in any git repository"
    exit 1
fi

JNUM=`cat /proc/cpuinfo | grep processor | wc -l`

if [ -f ~/.bashrc ]; then
    . ~/.bashrc
fi

branch=`git status | grep "On branch" | sed -r 's/.*On branch //g'`
cd $(git rev-parse --show-toplevel)

#scons tmp
aos make clean > /dev/null 2>&1
for target in ${scons_build_targets}; do
    if [ "${DEBUG}" != "no" ]; then
        echo "before scons ${target}@${branch}"
        pwd && ls
    fi
    aos scons ${target} > ${target}@${branch}.log 2>&1
    ret=$?
    if [ "${DEBUG}" != "no" ]; then
        echo "after scons ${target}@${branch}"
        pwd && ls
    fi
    if [ ${ret} -eq 0 ]; then
        echo "build scons ${target} at ${branch} branch succeed"
        rm -f ${target}@${branch}.log
    else
        echo -e "build scons ${target} at ${branch} branch failed, log:\n"
        cat ${target}@${branch}.log
        echo -e "\nbuild ${target} at ${branch} branch failed"
        aos make clean > /dev/null 2>&1
        exit 1
    fi
done

#tarsfer test
aos make clean > /dev/null 2>&1
for target in ${scons_ide_targets}; do
    for ide in ${ide_types}; do
        if [ "${DEBUG}" != "no" ]; then
            echo "before scons ${target} IDE=${ide} @${branch}"
            pwd && ls
        fi
        aos scons ${target} IDE=${ide} > ${target}2IDE_${ide}@${branch}.log 2>&1
        ret=$?
        if [ "${DEBUG}" != "no" ]; then
            echo "after scons ${target} IDE=${ide} @${branch}"
            pwd && ls
        fi
        if [ ${ret} -eq 0 ]; then
            echo "build scons ${target} IDE=${ide} at ${branch} branch succeed"
            rm -f ${target}2IDE_${ide}@${branch}.log
        else
            echo -e "build scons ${target} IDE=${ide} at ${branch} branch failed, log:\n"
            cat ${target}2IDE_${ide}@${branch}.log
            echo -e "\nbuild scons ${target} IDE=${ide} at ${branch} branch failed"
            aos make clean > /dev/null 2>&1
            exit 1
        fi
    done
done

#linuxhost posix
aos make clean > /dev/null 2>&1
for target in ${linux_posix_targets}; do
    for platform in ${linux_platforms}; do
        if [ "${DEBUG}" != "no" ]; then
            echo "before make posix ${target}@${platform}@${branch}"
            pwd && ls
        fi
        vcall=posix aos make ${target}@${platform} JOBS=${JNUM} > ${target}@${platform}@${branch}.log 2>&1
        ret=$?
        if [ "${DEBUG}" != "no" ]; then
            echo "after make posix ${target}@${platform}@${branch}"
            pwd && ls
        fi
        if [ ${ret} -eq 0 ]; then
            echo "build vcall=posix ${target}@${platform} at ${branch} branch succeed"
            rm -f ${target}@${platform}@${branch}.log
        else
            echo -e "build vcall=posix ${target}@${platform} at ${branch} branch failed, log:\n"
            cat ${target}@${platform}@${branch}.log
            echo -e "\nbuild ${target}@${platform} at ${branch} branch failed"
            aos make clean > /dev/null 2>&1
            exit 1
        fi
    done
done

#linuxhost
aos make clean > /dev/null 2>&1
for target in ${linux_targets}; do
    for platform in ${linux_platforms}; do
        if [ "${DEBUG}" != "no" ]; then
            echo "before make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        aos make ${target}@${platform} JOBS=${JNUM} > ${target}@${platform}@${branch}.log 2>&1
        ret=$?
        if [ "${DEBUG}" != "no" ]; then
            echo "after make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        if [ ${ret} -eq 0 ]; then
            echo "build ${target}@${platform} at ${branch} branch succeed"
            rm -f ${target}@${platform}@${branch}.log
        else
            echo -e "build ${target}@${platform} at ${branch} branch failed, log:\n"
            cat ${target}@${platform}@${branch}.log
            echo -e "\nbuild ${target}@${platform} at ${branch} branch failed"
            aos make clean > /dev/null 2>&1
            exit 1
        fi
    done
done

#single-bin, mk3060
aos make clean > /dev/null 2>&1
for target in ${mk3060_targets}; do
    for platform in ${mk3060_platforms}; do
        if [ "${DEBUG}" != "no" ]; then
            echo "before make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        aos make ${target}@${platform} JOBS=${JNUM} > ${target}@${platform}@${branch}.log 2>&1
        ret=$?
        if [ "${DEBUG}" != "no" ]; then
            echo "after make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        if [ ${ret} -eq 0 ]; then
            rm -f ${target}@${platform}@${branch}.log
            echo "build ${target}@${platform} at ${branch} branch succeed"
        else
            echo -e "build ${target}@${platform} at ${branch} branch failed, log:\n"
            cat ${target}@${platform}@${branch}.log
            rm -f ${target}@${platform}@${branch}.log
            echo -e "\nbuild ${target}@${platform} at ${branch} branch failed"
            aos make clean > /dev/null 2>&1
            exit 1
        fi
    done
done

#multi-bins, mk3060
aos make clean > /dev/null 2>&1
for target in ${mk3060_targets}; do
    for platform in ${mk3060_platforms}; do
        for bins in ${bins_type}; do
            if [ "${target}" = "tls" ] || [ "${target}" = "meshapp" ]; then
                continue
            fi
            if [ "${DEBUG}" != "no" ]; then
                echo "before make ${target}@${platform}@${bins}@${branch}"
                pwd && ls
            fi
            aos make ${target}@${platform} BINS=${bins} JOBS=${JNUM} > ${target}@${platform}@${bins}@${branch}.log 2>&1
            ret=$?
            if [ "${DEBUG}" != "no" ]; then
                echo "after make ${target}@${platform}@${bins}@${branch}"
                pwd && ls
            fi
            if [ ${ret} -eq 0 ]; then
                rm -f ${target}@${platform}@${bins}@${branch}.log
                echo "build ${target}@${platform} BINS=${bins} as multiple BINs at ${branch} branch succeed"
            else
                echo -e "build ${target}@${platform} BINS=${bins} as multiple BINs at ${branch} branch failed, log:\n"
                cat ${target}@${platform}@${bins}@${branch}.log
                rm -f ${target}@${platform}@${bins}@${branch}.log
                echo -e "\nbuild ${target}@${platform} BINS=${bins} as multiple BINs at ${branch} branch failed"
                aos make clean > /dev/null 2>&1
                exit 1
            fi
        done
    done
done

#single-bin, lpc54102
aos make clean > /dev/null 2>&1
for target in ${lpcxpresso54102_targets}; do
    for platform in ${lpcxpresso54102_platforms}; do
        if [ "${DEBUG}" != "no" ]; then
            echo "before make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        aos make ${target}@${platform} JOBS=${JNUM} > ${target}@${platform}@${branch}.log 2>&1
        ret=$?
        if [ "${DEBUG}" != "no" ]; then
            echo "after make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        if [ ${ret} -eq 0 ]; then
            rm -f ${target}@${platform}@${branch}.log
            echo "build ${target}@${platform} at ${branch} branch succeed"
        else
            echo -e "build ${target}@${platform} at ${branch} branch failed, log:\n"
            cat ${target}@${platform}@${branch}.log
            rm -f ${target}@${platform}@${branch}.log
            echo -e "\nbuild ${target}@${platform} at ${branch} branch failed"
            aos make clean > /dev/null 2>&1
            exit 1
        fi
    done
done

#single-bin, b_l475e
aos make clean > /dev/null 2>&1
for target in ${b_l475e_targets}; do
    for platform in ${b_l475e_platforms}; do
        if [ "${DEBUG}" != "no" ]; then
            echo "before make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        aos make ${target}@${platform} JOBS=${JNUM} > ${target}@${platform}@${branch}.log 2>&1
        ret=$?
        if [ "${DEBUG}" != "no" ]; then
            echo "after make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        if [ ${ret} -eq 0 ]; then
            rm -f ${target}@${platform}@${branch}.log
            echo "build ${target}@${platform} at ${branch} branch succeed"
        else
            echo -e "build ${target}@${platform} at ${branch} branch failed, log:\n"
            cat ${target}@${platform}@${branch}.log
            rm -f ${target}@${platform}@${branch}.log
            echo -e "\nbuild ${target}@${platform} at ${branch} branch failed"
            aos make clean > /dev/null 2>&1
            exit 1
        fi
    done
done

#single-bin, esp32
aos make clean > /dev/null 2>&1
for target in ${esp32_targets}; do
    for platform in ${esp32_platforms}; do
        if [ "${DEBUG}" != "no" ]; then
            echo "before make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        aos make -e ${target}@${platform} wifi=1 JOBS=${JNUM} > ${target}@${platform}@${branch}.log 2>&1
        ret=$?
        if [ "${DEBUG}" != "no" ]; then
            echo "after make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        if [ ${ret} -eq 0 ]; then
            rm -f ${target}@${platform}@${branch}.log
            echo "build ${target}@${platform} at ${branch} branch succeed"
        else
            echo -e "build ${target}@${platform} at ${branch} branch failed, log:\n"
            cat ${target}@${platform}@${branch}.log
            rm -f ${target}@${platform}@${branch}.log
            echo -e "\nbuild ${target}@${platform} at ${branch} branch failed"
            aos make clean > /dev/null 2>&1
            exit 1
        fi
    done
done

#single-bin, esp8266
aos make clean > /dev/null 2>&1
for target in ${esp8266_targets}; do
    for platform in ${esp8266_platforms}; do
        if [ "${DEBUG}" != "no" ]; then
            echo "before make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        aos make -e ${target}@${platform} wifi=1 JOBS=${JNUM} > ${target}@${platform}@${branch}.log 2>&1
        ret=$?
        if [ "${DEBUG}" != "no" ]; then
            echo "after make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        if [ ${ret} -eq 0 ]; then
            rm -f ${target}@${platform}@${branch}.log
            echo "build ${target}@${platform} at ${branch} branch succeed"
        else
            echo -e "build ${target}@${platform} at ${branch} branch failed, log:\n"
            cat ${target}@${platform}@${branch}.log
            rm -f ${target}@${platform}@${branch}.log
            echo -e "\nbuild ${target}@${platform} at ${branch} branch failed"
            aos make clean > /dev/null 2>&1
            exit 1
        fi
    done
done

#single-bin, mk3239
aos make clean > /dev/null 2>&1
for target in ${mk3239_targets}; do
    for platform in ${mk3239_platforms}; do
        if [ "${DEBUG}" != "no" ]; then
            echo "before make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        echo "Building mk3239"
        aos make ${target}@${platform} JOBS=${JNUM} > ${target}@${platform}@${branch}.log 2>&1
        ret=$?
        if [ "${DEBUG}" != "no" ]; then
            echo "after make ${target}@${platform}@${branch}"
            pwd && ls
        fi
        if [ ${ret} -eq 0 ]; then
            rm -f ${target}@${platform}@${branch}.log
            echo "build ${target}@${platform} at ${branch} branch succeed"
        else
            echo -e "build ${target}@${platform} at ${branch} branch failed, log:\n"
            cat ${target}@${platform}@${branch}.log
            rm -f ${target}@${platform}@${branch}.log
            echo -e "\nbuild ${target}@${platform} at ${branch} branch failed"
            aos make clean > /dev/null 2>&1
            exit 1
        fi
    done
done

aos make clean > /dev/null 2>&1
echo "build ${branch} branch succeed"

echo "----------check CODE-STYLE now----------"
#./build/astyle.sh
echo "----------------------------------------"
echo "----------check COPYRIGHT now-----------"
python ./build/copyright.py
echo "----------------------------------------"
