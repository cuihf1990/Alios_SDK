#!/bin/sh

workdir=~/testbed/autoscripts/
tests="example_test.py line_topology_test.py"
#recipients="apsaras73@list.alibaba-inc.com"
recipients="lc122798@alibaba-inc.com"
logfile=log.txt
success_num=0
fail_num=0

cd ${workdir}
echo "AOS automatic test start\n" > ${logfile}
for test in ${tests}; do
    echo "\n---------------------------------------------------------\n" >> ${logfile}
    echo "start ${test}\n" >> ${logfile}
    python ${test} >> ${logfile} 2>&1
    if [ $? -eq 0 ]; then
        ret="success"
        success_num=$((success_num+1))
    else
        ret="fail"
        fail_num=$((fail_num+1))
    fi
    echo "\nfinished ${test}, result=${ret}" >> ${logfile}
done
echo "\n---------------------------------------------------------\n" >> ${logfile}
echo "AOS automactic test finished, SUCCESS:${success_num}, FAIL:${fail_num}\n" >> ${logfile}

#send email
mailcontent=`cat ${logfile}`
echo -e ${mailcontent} | mutt -s "AOS automatic test result" -- ${recipients}
echo -e ${mailcontent} > ${logfile}

