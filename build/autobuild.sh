#!/bin/sh

workdir=autobuild
mk3060_targets="alinkapp helloworld linuxapp meshapp tls"
linux_targets="alinkapp helloworld linuxapp meshapp tls yts"
mk3060_platforms="mk3060 mk3060@release"
linux_platforms="linuxhost linuxhost@debug linuxhost@release"

git status > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "error: not in any git repository"
    exit 1
fi

branch=`git status | grep "On branch" | awk '{print $3}'`
cd $(git rev-parse --show-toplevel)
yos make clean > /dev/null 2>&1

for target in ${mk3060_targets}; do
    for platform in ${mk3060_platforms}; do
        yos make ${target}@${platform} > ${target}@${platform}@${branch}.log 2>&1
        if [ -f out/${target}@${platform}/binary/${target}@${platform}.elf ]; then
            rm -rf ${target}@${platform}@${branch}.log
            echo "build ${target}@${platform} at ${branch} branch succeed"
        else
            echo -e "\n"
            cat ${target}@${platform}@${branch}.log
            rm -rf ${target}@${platform}@${branch}.log
            echo -e "\nbuild ${target}@${platform} at ${branch} branch failed"
            yos make clean > /dev/null 2>&1
            exit 1
        fi
    done
done

for target in ${linux_targets}; do
    for platform in ${linux_platforms}; do
        yos make ${target}@${platform} > ${target}@${platform}@${branch}.log 2>&1
        if [ -f out/${target}@${platform}/binary/${target}@${platform}.elf ]; then
            echo "build ${target}@${platform} at ${branch} branch succeed"
            rm -rf ${target}@${platform}@${branch}.log
        else
            echo -e "\n"
            cat ${target}@${platform}@${branch}.log
            echo -e "\nbuild ${target}@${platform} at ${branch} branch failed"
            yos make clean > /dev/null 2>&1
            exit 1
        fi
    done
done

yos make clean > /dev/null 2>&1
echo "build ${branch} branch succeed"
