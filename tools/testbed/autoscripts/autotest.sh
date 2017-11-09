#!/bin/bash

mesh_tests="line_topology_v4_test.py tree_topology_v4_test.py mcast_v4_test.py"
recipients="apsaras73@list.alibaba-inc.com"
meshrecipients="wanglu.luwang@alibaba-inc.com wenjunchen.cwj@alibaba-inc.com simen.cjj@alibaba-inc.com lc122798@alibaba-inc.com"
#recipients="lc122798@alibaba-inc.com"
#meshrecipients="lc122798@alibaba-inc.com"
logfile=~/auto_test_log.txt
success_num=0
fail_num=0
mk3060firmware=""
esp32firmware=""

#Usage: bash autotest.sh mk3060_firmware esp32_firmware
if [ $# -gt 0 ]; then
    mk3060firmware=$1
fi
if [ $# -gt 1 ]; then
    esp32firmware=$2
fi

echo -e "Start Alink 2PPS and 5PPS test\n" > ${logfile}
alinkpids=""
prefix=
if [ -f ${mk3060firmware} ]; then
    model=mk3060
    python alink_testrun.py --testname=${prefix}5pps --firmware=${mk3060firmware} --model=${model} > ~/${model}_5pps.txt 2>&1 &
    mk3060_5ppsPid=$!
    sleep 30
    python alink_testrun.py --testname=${prefix}2pps --firmware=${mk3060firmware} --model=${model} > ~/${model}_2pps.txt 2>&1 &
    mk3060_2ppsPid=$!
    alinkpids="${alinkpids} ${mk3060_5ppsPid} ${mk3060_2ppsPid}"
fi
if [ -f ${esp32firmware} ]; then
    model=esp32
    python alink_testrun.py --testname=${prefix}5pps --firmware=${esp32firmware} --model=${model} > ~/${model}_5pps.txt 2>&1 &
    esp32_5ppsPid=$!
    sleep 30
    python alink_testrun.py --testname=${prefix}2pps --firmware=${esp32firmware} --model=${model} > ~/${model}_2pps.txt 2>&1 &
    esp32_2ppsPid=$!
    alinkpids="${alinkpids} ${esp32_5ppsPid} ${esp32_2ppsPid}"
fi

if [ -f ${mk3060firmware} ]; then
    model="mk3060"
    echo -e "Start AOS functional automatic test with ${model}\n" >> ${logfile}
    for test in ${mesh_tests}; do
        echo -e "\n---------------------------------------------------------\n" >> ${logfile}
        echo -e "start ${test}\n" >> ${logfile}
        python ipv4/${test} --firmware=${mk3060firmware} >> ${logfile} 2>&1
        if [ $? -eq 0 ]; then
            ret="success"
            success_num=$((success_num+1))
        else
            ret="fail"
            fail_num=$((fail_num+1))
        fi
        echo -e "\nfinished ${test}, result=${ret}" >> ${logfile}
    done
    echo -e "\n---------------------------------------------------------\n" >> ${logfile}
    echo -e "AOS functional automactic test with ${model} finished, SUCCESS:${success_num}, FAIL:${fail_num}\n" >> ${logfile}

    #send result to mesh group first
    title="Mesh ${model} autotest: SUCCESS-${success_num}, FAIL-${fail_num}"
    cat ${logfile} | mutt -s "${title}" -- ${meshrecipients}
fi

running=""
for pid in ${alinkpids}; do
    running=`ps | grep ${pid}`
    if [ "${running}" != "" ]; then
        break
    fi
done
while [ "${running}" != "" ]; do
    sleep 60
    running=""
    for pid in ${alinkpids}; do
        running=`ps | grep ${pid}`
        if [ "${running}" != "" ]; then
            break
        fi
    done
done


title=""
if [ -f ${mk3060firmware} ]; then
    wait ${mk3060_5ppsPid}
    mk3060_5ppsRet=$?
    wait ${mk3060_2ppsPid}
    mk3060_2ppsRet=$?
    echo -e "\n---------------------------------------------------------\n" >> ${logfile}
    if [ ${mk3060_5ppsRet} -eq 0 ]; then
        echo -e "run Alink 5PPS test with mk3060 succeed, log:\n" >> ${logfile}
        title="${title} mk3060: 5PPS-SUCCESS,"
    else
        echo -e "run Alink 5PPS test with mk3060 failed, log:\n" >> ${logfile}
        title="${title} mk3060: 5PPS-FAIL,"
    fi
    cat ~/mk3060_5pps.txt >> ${logfile}
    rm -f ~/mk3060_5pps.txt

    echo -e "\n---------------------------------------------------------\n" >> ${logfile}
    if [ ${mk3060_2ppsRet} -eq 0 ]; then
        echo -e "run Alink 2PPS test with mk3060 succeed, log:\n" >> ${logfile}
        title="${title} 2PPS-SUCCESS;"
    else
        echo -e "run Alink 2PPS test with mk3060 failed, log:\n" >> ${logfile}
        title="${title} 2PPS-FAIL;"
    fi
    cat ~/mk3060_2pps.txt >> ${logfile}
    rm -f ~/mk3060_2pps.txt
fi

if [ -f ${esp32firmware} ]; then
    wait ${esp32_5ppsPid}
    esp32_5ppsRet=$?
    wait ${esp32_2ppsPid}
    esp32_2ppsRet=$?
    echo -e "\n---------------------------------------------------------\n" >> ${logfile}
    if [ ${esp32_5ppsRet} -eq 0 ]; then
        echo -e "run Alink 5PPS test with esp32 succeed, log:\n" >> ${logfile}
        title="${title} esp32: 5PPS-SUCCESS,"
    else
        echo -e "run Alink 5PPS test with esp32 failed, log:\n" >> ${logfile}
        title="${title} 5PPS-FAIL,"
    fi
    cat ~/esp32_5pps.txt >> ${logfile}
    rm -f ~/esp32_5pps.txt

    echo -e "\n---------------------------------------------------------\n" >> ${logfile}
    if [ ${esp32_2ppsRet} -eq 0 ]; then
        echo -e "run Alink 2PPS test with esp32 succeed, log:\n" >> ${logfile}
        title="${title} 2PPS-SUCCESS;"
    else
        echo -e "run Alink 2PPS test with esp32 failed, log:\n" >> ${logfile}
        title="${title} 2PPS-FAIL;"
    fi
    cat ~/esp32_2pps.txt >> ${logfile}
    rm -f ~/esp32_2pps.txt
fi

#send email
title="Autotest: mesh SUCCESS-${success_num}, FAIL-${fail_num};${title}"
cat ${logfile} | mutt -s "${title}" -- ${recipients}
rm -f ${logfile}
