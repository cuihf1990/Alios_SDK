#!/bin/sh

cd $(git rev-parse --show-toplevel)
aosfolder=`pwd`

if [ -f sync ] || [ -d sync ]; then
    rm -rf sync
fi
mkdir sync

git checkout master
if [ $? -ne 0 ]; then
    echo "error: swith to master branch failed"
    exit 1
fi

aos make meshapp@linuxhost meshdebug=1
if [ $? -ne 0 ]; then
    echo "error: build libraries for linuxhost failed"
    exit 1
fi

aos make alinkapp@mk3060 meshdebug=1
if [ $? -ne 0]; then
    echo "error: build libraries for mk3060 failed"
    exit 1
fi

cd ${aosfolder}/out/meshapp@linuxhost/libraries/
cp mesh.a libmesh-linuxhost.a
strip --strip-debug libmesh-linuxhost.a
mv libmesh-linuxhost.a ${aosfolder}/sync/

cd ${aosfolder}/out/alinkapp@mk3060/libraries/
echo "create libbeken.a" > packscript
echo "addlib beken.a" >> packscript
echo "addlib entry.a" >> packscript
echo "addlib hal_init.a" >> packscript
echo "addlib ${aosfolder}/platform/mcu/beken/librwnx.a" >> packscript
echo "save" >> packscript
echo "end" >> packscript
arm-none-eabi-ar -M < packscript
arm-none-eabi-strip --strip-debug libbeken.a
mv libbeken.a ${aosfolder}/sync/

cp mesh.a libmesh-mk3060.a
arm-none-eabi-strip --strip-debug libmesh-mk3060.a
mv libmesh-mk3060.a ${aosfolder}/sync/

cd ${aosfolder}
syncbranch=`git branch | grep githubsync`
if [ "${syncbranch}" == "" ]; then
    git branch --track githubsync origin/githubsync
fi
git checkout githubsync
cp sync/libmesh-linuxhost.a kernel/protocols/mesh/lib/linuxhost/libmesh.a
cp sync/libmesh-mk3060.a kernel/protocols/mesh/lib/mk3060/libmesh.a
cp sync/libbeken.a platform/mcu/beken/
rm -rf sync
git add -A
datetime=`date +%F@%H:%M`
git commit -m "update libraries at ${datetime}"
git push -f origin githubsync
