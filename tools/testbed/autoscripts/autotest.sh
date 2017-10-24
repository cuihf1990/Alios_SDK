#!/bin/sh

workdir=`pwd`
tests="line_topology_v4_test.py tree_topology_v4_test.py mcast_v4_test.py"
recipients="apsaras73@list.alibaba-inc.com"
meshrecipients="wanglu.luwang@alibaba-inc.com wenjunchen.cwj@alibaba-inc.com simen.cjj@alibaba-inc.com lc122798@alibaba-inc.com"
logfile=~/auto_test_log.txt
log2pps=~/2pps_test_log.txt
log5pps=~/5pps_test_log.txt
success_num=0
fail_num=0
firmware=master.bin

cd ${workdir}

if [ $# -gt 0 ] && [ -f $1 ]; then
    firmware=$1
fi

echo -e "Start Alink 2PPS and 5PPS test\n" >> ${logfile}
python alink_testrun.py --testname=2pps --device=mxchip-DN02X304 --firmware=${firmware} --caseid=24015 > ${log2pps} 2>&1 &
pps2pid=$!
sleep 60
python alink_testrun.py --testname=5pps --device=mxchip-DN02X30B --firmware=${firmware} --caseid=24029 > ${log5pps} 2>&1 &
pps5pid=$!

cd ${workdir}/ipv4
echo -e "Start AOS functional automatic test\n" >> ${logfile}
for test in ${tests}; do
    echo -e "\n---------------------------------------------------------\n" >> ${logfile}
    echo -e "start ${test}\n" >> ${logfile}
    python ${test} --firmware=${workdir}/${firmware} >> ${logfile} 2>&1
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

#send result to mesh group first
title="Mesh Automatic Test Result: SUCCESS-${success_num}, FAIL-${fail_num}"
cat ${logfile} | mutt -s "${title}" -- ${meshrecipients}

cd ${workdir}
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
cat ${log2pps} >> ${logfile}
rm -f ${log2pps}

echo -e "\n---------------------------------------------------------\n" >> ${logfile}
if [ ${pps5ret} -eq 0 ]; then
    echo -e "run Alink 5PPS test succeed, log:\n" >> ${logfile}
    title="${title}; 5PPS-SUCCESS"
else
    echo -e "run Alink 5PPS test failed, log:\n" >> ${logfile}
    title="${title}; 5PPS-FAIL"
fi
cat ${log5pps} >> ${logfile}
rm -f ${log5pps}

#send email
title="AOS Automatic Test Result: mesh SUCCESS-${success_num}, FAIL-${fail_num}; ${title}"
cat ${logfile} | mutt -s "${title}" -- ${recipients}
rm -f ${logfile}

