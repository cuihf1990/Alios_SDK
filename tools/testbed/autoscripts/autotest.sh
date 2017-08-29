#!/bin/sh

workdir=~/testbed/autoscripts/
tests="example_test.py line_topology_test.py tree_topology_test.py mixed_topology_test.py mcast_test.py"
recipients="apsaras73@list.alibaba-inc.com"
#recipients="lc122798@alibaba-inc.com"
logfile=log.txt
success_num=0
fail_num=0
firmware=master.bin

cd ${workdir}

if [ $# -gt 0 ] && [ -f $1 ]; then
    firmware=$1
fi

echo -e "Start programing devices, firmware=${firmware}\n" > ${logfile}
python testbed_program.py ${firmware} >> ${logfile} 2>&1
echo -e "\n---------------------------------------------------------\n" >> ${logfile}

echo -e "Start Alink 2PPS and 5PPS test\n" >> ${logfile}
python alink_testrun.py --testname=2pps --device=mxchip-DN02XLNN --filename=${firmware} --caseid=15071 --userid=500001169232518525 --server=pre-iotx-qs.alibaba.com --port=80 --wifissid=aos_test_01 --wifipass=Alios@Embedded > 2pps.log 2>&1 &
pps2pid=$!
sleep 60
python alink_testrun.py --testname=5pps --device=mxchip-DN02X30I --filename=${firmware} --caseid=15100 --userid=500001169232518525 --server=pre-iotx-qs.alibaba.com --port=80 --wifissid=aos_test_01 --wifipass=Alios@Embedded > 5pps.log 2>&1 &
pps5pid=$!

echo -e "Start AOS functional automatic test\n" >> ${logfile}
for test in ${tests}; do
    echo -e "\n---------------------------------------------------------\n" >> ${logfile}
    echo -e "start ${test}\n" >> ${logfile}
    python ${test} >> ${logfile} 2>&1
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
echo -e "AOS functional automactic test finished, SUCCESS:${success_num}, FAIL:${fail_num}\n" >> ${logfile}

pps2status=`ps | grep ${pps2pid}`
pps5status=`ps | grep ${pps5pid}`
while [ "$pps2status" != "" ] || [ "${pps5status}" != "" ]; do
    sleep 60
    pps2status=`ps | grep ${pps2pid}`
    pps5status=`ps | grep ${pps5pid}`
done

wait ${pps2pid}
pps2ret=$?
wait ${pps5pid}
pps5ret=$?

echo -e "\n---------------------------------------------------------\n" >> ${logfile}
if [ ${pps2ret} -eq 0 ]; then
    echo -e "run Alink 2PPS test succeed, log:\n" >> ${logfile}
    title="2PPS-SUCCESS"
else
    echo -e "run Alink 2PPS test failed, log:\n" >> ${logfile}
    title="2PPS-FAIL"
fi
cat 2pps.log >> ${logfile}
rm -f 2pps.log

echo -e "\n---------------------------------------------------------\n" >> ${logfile}
if [ ${pps5ret} -eq 0 ]; then
    echo -e "run Alink 5PPS test succeed, log:\n" >> ${logfile}
    title="${title}; 5PPS-SUCCESS"
else
    echo -e "run Alink 5PPS test failed, log:\n" >> ${logfile}
    title="${title}; 5PPS-FAIL"
fi
cat 5pps.log >> ${logfile}
rm -f 5pps.log

#send email
title="AOS Automatic Test Result: mesh SUCCESS-${success_num}, FAIL-${fail_num}; ${title}"
cat ${logfile} | mutt -s "${title}" -- ${recipients}

