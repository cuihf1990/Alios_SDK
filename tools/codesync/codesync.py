import sys
import traceback
import subprocess
import os
import errno
import argparse

def print_help():
    print  "help"

def log(msg):
    sys.stdout.write(msg)
    sys.stdout.flush()

def message(msg):
    return "[aos] %s\n" % msg

def info(msg, level=1):
    if level <= 0:
        for line in msg.splitlines():
            log(message(line))

class ProcessException(Exception):
    pass

def popen(command, stdin=None, **kwargs):
    info('Exec "'+' '.join(command)+'" in '+os.getcwd())
    try:
        proc = subprocess.Popen(command, **kwargs)
    except OSError as e:
        if e[0] == errno.ENOENT:
            error(
                "Could not execute \"%s\".\n"
                "Please verify that it's installed and accessible from your current path by executing \"%s\".\n" % (command[0], command[0]), e[0])
        else:
            raise e
    if proc.wait() != 0:
        raise ProcessException(proc.returncode, command[0], ' '.join(list(command)), os.getcwd())

class syncpreparelib():
    def __init__(self, srcbase, dstbase):
        mac = ""
        win = ""

        if os.path.exists("./tmp_synccode"):
            linux= "rm -rf tmp_synccode"
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            popen(cmd, shell=True, cwd=os.getcwd())

        linux= "mkdir tmp_synccode"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())
        cur_dir = os.getcwd()
        os.chdir("./tmp_synccode")

        if srcbase == "master":
            git_cmd = "git clone git@code.aliyun.com:keepwalking.zeng/aos.git"
            popen(git_cmd, shell=True, cwd=os.getcwd())
            os.chdir("./aos")
            self.srcdir = "./aos"

            linux = "aos make meshapp@linuxhost"
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            popen(cmd, shell=True, cwd=os.getcwd())

            linux = "aos make alinkapp@linuxhost"
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            popen(cmd, shell=True, cwd=os.getcwd())

            linux = "aos make alinkapp@mk3060"
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            popen(cmd, shell=True, cwd=os.getcwd())
        else:
            error('Unknown source base!')

        os.chdir(cur_dir)
        os.chdir("./tmp_synccode")
        if dstbase == "github":
            #git_cmd = "git clone git@github.com:alibaba/AliOS-Things.git"
            git_cmd = "mkdir AliOS-Things"
            popen(git_cmd, shell=True, cwd=os.getcwd())
            os.chdir("./AliOS-Things")
            self.dstdir = "./AliOS-Things"
        else:
            error('Unknown source base!')

        os.chdir(cur_dir)
        os.chdir("./tmp_synccode")

    def cleanup_codebase(self):
        mac = ""
        win = ""

        dst = self.dstdir + "/*"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/.gitignore"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/.vscode"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/*"
        dst = self.dstdir
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/.gitignore"
        dst = self.dstdir
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/.vscode"
        dst = self.dstdir
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

    def cleanup_boards(self):
        mac = ""
        win = ""

        dst = self.dstdir + "/board/armhflinux"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/board/mk108"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

    def cleanup_build(self):
        mac = ""
        win = ""

        dst = self.dstdir + "/build/astyle"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/build/astyle.sh"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/build/copyright.py"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/build/copyright"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/build/doxygen2md.py"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/build/MD.templet"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/build/OpenOCD"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/build/compiler/arm-none-eabi*"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

    def cleanup_script(self):
        mac = ""
        win = ""

        dst = self.dstdir + "/script"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

    def cleanup_platform(self):
        mac = ""
        win = ""

        dst = self.dstdir + "/platform/mcu/liux/csp/wifi/"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/platform/arch/linux/swap.*"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

    def cleanup_kernel(self):
        mac = ""
        win = ""

        dst = self.dstdir + "/kernel/rhino/test"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

    def cleanup_example(self):
        mac = ""
        win = ""

        dst = self.dstdir + "/example/mqttest"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/example/tls"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/example/yts"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

    def cleanup_bootloader(self):
        mac = ""
        win = ""

        dst = self.dstdir + "/bootloader"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

    def cleanup_test(self):
        mac = ""
        win = ""

        dst = self.dstdir + "/test"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

    def cleanup_tools(self):
        mac = ""
        win = ""

        dst = self.dstdir + "/tools/*"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/tools/Doxyfile"
        dst = self.dstdir + "/tools"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/tools/doxygen.sh"
        dst = self.dstdir + "/tools"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/tools/cli"
        dst = self.dstdir + "/tools"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

class meshlib:
    def __init__(self, srcdir, dstdir):
        self.srcdir = srcdir
        self.dstdir = dstdir

    def make_folder(self):
        mac = ""
        win = ""

        dst = self.dstdir + "/kernel/protocols/mesh/*"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/kernel/protocols/mesh/lib"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/kernel/protocols/mesh/lib/linuxhost"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/kernel/protocols/mesh/lib/mk3060"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/kernel/protocols/mesh/include"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/kernel/protocols/mesh/include/umesh_80211.h"
        dst = self.dstdir + "/kernel/protocols/mesh/include/"
        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/kernel/protocols/mesh/include/umesh.h"
        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/kernel/protocols/mesh/include/umesh_hal.h"
        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/kernel/protocols/mesh/include/umesh_types.h"
        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = "~/aos/tools/codesync/modules/mesh.mk"
        dst = self.dstdir + "/kernel/protocols/mesh/"
        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

    def make_lib(self):
        mac = ""
        win = ""
        src = self.srcdir + "/out/alinkapp@mk3060/libraries/mesh.a"
        dst = self.dstdir + "/kernel/protocols/mesh/lib/mk3060/libmesh.a"
        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/out/meshapp@linuxhost/libraries/mesh.a"
        dst = self.dstdir + "/kernel/protocols/mesh/lib/linuxhost/libmesh.a"
        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

class ywsslib:
    def __init__(self, srcdir, dstdir):
        self.srcdir = srcdir
        self.dstdir = dstdir

    def make_folder(self):
        win = ""
        mac = ""
        dst = self.dstdir + "/framework/ywss/*"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/framework/ywss/lib"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/framework/ywss/lib/linuxhost"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/framework/ywss/lib/mk3060"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/framework/ywss/awss.h"
        dst = self.dstdir + "/framework/ywss/"
        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/framework/ywss/enrollee.h"
        dst = self.dstdir + "/framework/ywss/"
        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = "~/aos/tools/codesync/modules/ywss.mk"
        dst = self.dstdir + "/framework/ywss/"
        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

    def make_lib(self):
        mac = ""
        win = ""
        src = self.srcdir + "/out/alinkapp@mk3060/libraries/ywss.a"
        dst = self.dstdir + "/framework/ywss/lib/mk3060/libywss.a"
        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/out/alinkapp@linuxhost/libraries/ywss.a"
        dst = self.dstdir + "/framework/ywss/lib/linuxhost/libywss.a"
        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

class bekenlib:
    def __init__(self, srcdir, dstdir):
        self.srcdir = srcdir
        self.dstdir = dstdir

    def make_folder(self):
        win = ""
        mac = ""

        dst = self.dstdir + "/platform/mcu/beken/*"
        linux = "rm -rf " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/platform/mcu/beken/include"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/platform/mcu/beken/include/lwip-2.0.2"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/platform/mcu/beken/include/app"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/platform/mcu/beken/include/func"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/platform/mcu/beken/include/os"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/platform/mcu/beken/include/driver"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/platform/mcu/beken/include/ip"
        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/beken7231/beken378/func/mxchip/lwip-2.0.2/port"
        dst = self.dstdir + "/platform/mcu/beken/include/lwip-2.0.2"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/beken7231/beken378/common"
        dst = self.dstdir + "/platform/mcu/beken/include"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/beken7231/beken378/app/config"
        dst = self.dstdir + "/platform/mcu/beken/include/app"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/beken7231/beken378/func/include"
        dst = self.dstdir + "/platform/mcu/beken/include/func"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/beken7231/beken378/os/include"
        dst = self.dstdir + "/platform/mcu/beken/include/os/"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/beken7231/beken378/driver/include"
        dst = self.dstdir + "/platform/mcu/beken/include/driver"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/beken7231/beken378/driver/common"
        dst = self.dstdir + "/platform/mcu/beken/include/driver"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/beken7231/beken378/ip/common"
        dst = self.dstdir + "/platform/mcu/beken/include/ip"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/beken7231/beken378/driver/entry/*.h"
        dst = self.dstdir + "/platform/mcu/beken/include"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/beken7231/beken378/build"
        dst = self.dstdir + "/platform/mcu/beken/linkinfo"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/platform/mcu/beken"
        linux = "find " + dst + " -type f -name '*.c' -exec rm {} +"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/beken7231/beken378/build"
        dst = self.dstdir + "/platform/mcu/beken/linkinfo"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/aos"
        dst = self.dstdir + "/platform/mcu/beken/"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/hal"
        dst = self.dstdir + "/platform/mcu/beken/"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/encrypt_linux"
        dst = self.dstdir + "/platform/mcu/beken/"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/encrypt_osx"
        dst = self.dstdir + "/platform/mcu/beken/"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/encrypt_win.exe"
        dst = self.dstdir + "/platform/mcu/beken/"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/gen_crc_bin.mk"
        dst = self.dstdir + "/platform/mcu/beken/"
        linux = "cp -rf " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = "~/aos/tools/codesync/modules/beken.mk"
        dst = self.dstdir + "/platform/mcu/beken/"
        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

    def make_lib(self):
        build_tool = self.srcdir + "/build/compiler/arm-none-eabi-5_4-2016q2-20160622/Linux64/bin/"
        mac = ""
        win = ""

        linux = "echo \"create libbeken.a\" > packscript"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/out/alinkapp@mk3060/libraries/beken.a"
        dst = " ."
        linux = "cp " + src + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/out/alinkapp@mk3060/libraries/entry.a"
        dst = " ."
        linux = "cp " + src + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/out/alinkapp@mk3060/libraries/hal_init.a"
        dst = " ."
        linux = "cp " + src + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        src = self.srcdir + "/platform/mcu/beken/librwnx/librwnx.a"
        dst = " ."
        linux = "cp " + src + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        linux = "echo \"addlib beken.a\" >> packscript"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        linux = "echo \"addlib entry.a\" >> packscript"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        linux = "echo \"addlib hal_init.a\" >> packscript"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        linux = "echo \"addlib librwnx.a\" >> packscript"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        linux = "echo \"save\" >> packscript"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        linux = "echo \"end\" >> packscript"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        linux = build_tool + "arm-none-eabi-ar -M < packscript"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        linux = build_tool + "arm-none-eabi-strip --strip-debug libbeken.a"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/platform/mcu/beken"
        linux = "mv libbeken.a " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        dst = self.dstdir + "/platform/mcu/beken/"
        linux = build_tool + "arm-none-eabi-ar d " + dst + "aos_main.o"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        linux = build_tool + "arm-none-eabi-ar d " + dst + "soc_impl.o"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        linux = build_tool + "arm-none-eabi-ar d " + dst + "trace_impl.o"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        linux = build_tool + "arm-none-eabi-ar d " + dst + "mesh_wifi_hal.o"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

        linux = "rm *.a"
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
        popen(cmd, shell=True, cwd=os.getcwd())

parser = argparse.ArgumentParser(prog='codesync',
    description="Code sync tool for aos",
    formatter_class=argparse.RawTextHelpFormatter)
parser.add_argument("-s", "--srcbase", dest="srcbase", default='master')
parser.add_argument("-d", "--dstbase", dest="dstbase", default='github')
parser.add_argument("-m", "--module", dest="modules", default='mesh ywss beken')

def main():
    pargs, remainder = parser.parse_known_args(sys.argv[1:])
    syncprepare = syncpreparelib(pargs.srcbase, pargs.dstbase)
    syncprepare.cleanup_codebase()
    syncprepare.cleanup_boards()
    syncprepare.cleanup_build()
    syncprepare.cleanup_script()
    syncprepare.cleanup_platform()
    syncprepare.cleanup_kernel()
    syncprepare.cleanup_example()
    syncprepare.cleanup_bootloader()
    syncprepare.cleanup_test()
    syncprepare.cleanup_tools()
    modules = pargs.modules.split(" ")
    for module in modules:
        if module == "mesh":
            mesh = meshlib(syncprepare.srcdir, syncprepare.dstdir)
            mesh.make_folder()
            mesh.make_lib()
        elif module == "ywss":
            ywss = ywsslib(syncprepare.srcdir, syncprepare.dstdir)
            ywss.make_folder()
            ywss.make_lib()
        elif module == "beken":
            beken = bekenlib(syncprepare.srcdir, syncprepare.dstdir)
            beken.make_folder()
            beken.make_lib()
        else:
            print "unknown module", module

if __name__ == "__main__":
    main()
