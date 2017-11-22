#!/bin/bash

mesh_tests="line_topology_v4_test.py tree_topology_v4_test.py mcast_v4_test.py"
recipients="apsaras73@list.alibaba-inc.com"
meshrecipients="xiaoma.mx@alibaba-inc.com gubiao.dc@alibaba-inc.com yuanlu.gl@alibaba-inc.com wanglu.luwang@alibaba-inc.com wenjunchen.cwj@alibaba-inc.com simen.cjj@alibaba-inc.com lc122798@alibaba-inc.com"
#recipients="lc122798@alibaba-inc.com"
#meshrecipients="lc122798@alibaba-inc.com"
logfile=~/auto_test_log.txt
mesh_pass_total=0
mesh_fail_total=0
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
    mesh_pass=0
    mesh_fail=0
    echo -e "Start MESH functional autotest with ${model} ...\n" >> ${logfile}
    for test in ${mesh_tests}; do
        echo -e "\n---------------------------------------------------------\n" >> ${logfile}
        echo -e "start ${test}\n" >> ${logfile}
        python ${test} --firmware=${mk3060firmware} --model=${model} >> ${logfile} 2>&1
        if [ $? -eq 0 ]; then
            ret="success"
            mesh_pass_total=$((mesh_pass_total+1))
            mesh_pass=$((mesh_pass+1))
        else
            ret="fail"
            mesh_fail_total=$((mesh_fail_total+1))
            mesh_fail=$((mesh_fail+1))
        fi
        echo -e "\nfinished ${test}, result=${ret}" >> ${logfile}
    done
    echo -e "\n---------------------------------------------------------\n" >> ${logfile}
    echo -e "Finished MESH functional autotest with ${model}, PASS:${mesh_pass} FAIL:${mesh_fail}\n" >> ${logfile}

    title=" ${model}-PASS-${mesh_pass} FAIL-${mesh_fail};"
fi
if [ -f ${esp32firmware} ]; then
    model="esp32"
    mesh_pass=0
    mesh_fail=0
    echo -e "Start MESH functional autotest with ${model} ...\n" >> ${logfile}
    for test in ${mesh_tests}; do
        echo -e "\n---------------------------------------------------------\n" >> ${logfile}
        echo -e "start ${test}\n" >> ${logfile}
        python ${test} --firmware=${esp32firmware} --model={model} >> ${logfile} 2>&1
        if [ $? -eq 0 ]; then
            ret="success"
            mesh_pass_total=$((mesh_pass_total+1))
            mesh_pass=$((mesh_pass+1))
        else
            ret="fail"
            mesh_fail_total=$((mesh_fail_total+1))
            mesh_fail=$((mesh_fail+1))
        fi
        echo -e "\nfinished ${test}, result=${ret}" >> ${logfile}
    done
    echo -e "\n---------------------------------------------------------\n" >> ${logfile}
    echo -e "Finished MESH functional automactic test with ${model}, PASS:${mesh_pass} FAIL:${mesh_fail}\n" >> ${logfile}

    title=" ${model}-PASS:${mesh_pass} FAIL:${mesh_fail};"
fi

#send result to mesh group first
if [ -f ${mk3060firmware} ] || [ -f ${esp32firmware} ]; then
    title="Mesh Autotest:${title}"
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
        title="${title} MK3060: 5PPS-PASS"
    else
        echo -e "run Alink 5PPS test with mk3060 failed, log:\n" >> ${logfile}
        title="${title} MK3060: 5PPS-FAIL"
    fi
    cat ~/mk3060_5pps.txt >> ${logfile}
    rm -f ~/mk3060_5pps.txt

    echo -e "\n---------------------------------------------------------\n" >> ${logfile}
    if [ ${mk3060_2ppsRet} -eq 0 ]; then
        echo -e "run Alink 2PPS test with mk3060 succeed, log:\n" >> ${logfile}
        title="${title} 2PPS-PASS;"
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
        title="${title} ESP32: 5PPS-PASS"
    else
        echo -e "run Alink 5PPS test with esp32 failed, log:\n" >> ${logfile}
        title="${title} ESP32: 5PPS-FAIL"
    fi
    cat ~/esp32_5pps.txt >> ${logfile}
    rm -f ~/esp32_5pps.txt

    echo -e "\n---------------------------------------------------------\n" >> ${logfile}
    if [ ${esp32_2ppsRet} -eq 0 ]; then
        echo -e "run Alink 2PPS test with esp32 succeed, log:\n" >> ${logfile}
        title="${title} 2PPS-PASS;"
    else
        echo -e "run Alink 2PPS test with esp32 failed, log:\n" >> ${logfile}
        title="${title} 2PPS-FAIL;"
    fi
    cat ~/esp32_2pps.txt >> ${logfile}
    rm -f ~/esp32_2pps.txt
fi

#send email
title="Autotest: MESH: PASS-${mesh_pass_total} FAIL-${mesh_fail_total};${title}"
cat ${logfile} | mutt -s "${title}" -- ${recipients}
rm -f ${logfile}
