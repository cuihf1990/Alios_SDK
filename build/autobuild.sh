#!/usr/bin/env bash

workdir=autobuild
linux_posix_targets="alinkapp meshapp"
linux_targets="alinkapp helloworld linuxapp meshapp tls yts"
linux_platforms="linuxhost linuxhost@debug linuxhost@release"
mk3060_targets="alinkapp helloworld linuxapp meshapp tls uDataapp"
mk3060_platforms="mk3060 mk3060@release"
b_l475e_targets="mqttapp helloworld tls uDataapp"
b_l475e_platforms="b_l475e"
esp32_targets="alinkapp helloworld"
esp32_platforms="esp32devkitc"
bins_type="app framework kernel"
mk3239_targets="bluetooth.ble_advertisements bluetooth.ble_show_system_time"
mk3239_platforms="mk3239"

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
            if [ "${target}" = "tls" ] || [ "${target}" = "meshapp" ] || [ "${target}" = "uDataapp" ]; then
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
