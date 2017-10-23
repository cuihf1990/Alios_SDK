import sys
import traceback
import errno
import os
import modules

#########################################
# add supported chip vendors branch here
#########################################

class vendorslib:
    def __init__(self, srcbase, base):
        self.srcbase = srcbase
        self.base = base
        self.srcdir = "./aos"

    def get_vendor_repo(self):
        ret = 0
        if self.base == "github":
            self.git_cmd = "git clone git@github.com:alibaba/AliOS-Things.git"
            self.dstdir = "./AliOS-Things"
        elif self.base == "mxchip":
            self.git_cmd = "git clone git@code.aliyun.com:keepwalking.zeng/aos-pbase.git"
            self.dstdir = "./aos-pbase"
        elif self.base == "allwinner":
            self.git_cmd = "git clone git@code.aliyun.com:keepwalking.zeng/yunos-iot-project.git"
            self.dstdir = "./yunos-iot-project"
        elif self.base == "telink":
            self.git_cmd = "git clone git@code.aliyun.com:keepwalking.zeng/alios-nano.git"
            self.dstdir = "./alios-nano"
        else:
            ret = 1
        return ret

    def cleanup_code(self):
        win = ""
        mac = ""
        if self.base == "allwinner":
            ##############################################################
            # keep platform and board folder the same as different targets
            ##############################################################
            src = self.dstdir + "/platform"
            if os.path.exists(src):
                linux = "cp -rf " + src + " ./"
                cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
                if not cmd:
                    error('Unknown system!')
                modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.dstdir + "/board"
            if os.path.exists(src):
                linux = "cp -rf " + src + " ./"
                cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
                if not cmd:
                    error('Unknown system!')
                modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.dstdir + "/example"
            if os.path.exists(src):
                linux = "cp -rf " + src + " ./"
                cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
                if not cmd:
                    error('Unknown system!')
                modules.popen(cmd, shell=True, cwd=os.getcwd())

            dst = self.dstdir + "/*"
            linux = "rm -rf " + dst
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            dst = self.dstdir + "/.gitignore"
            if os.path.exists(dst):
                linux = "rm -rf " + dst
                cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
                if not cmd:
                    error('Unknown system!')
                modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/*"
            dst = self.dstdir
            linux = "cp -rf " + src + " " + dst
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            dst = self.dstdir + "/platform"
            linux = "rm -rf " + dst
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            dst = self.dstdir + "/board"
            linux = "rm -rf " + dst
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            dst = self.dstdir + "/example"
            linux = "rm -rf " + dst
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            if os.path.exists("./platform"):
                linux = "cp -rf " + "./platform "+ self.dstdir
                cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
                if not cmd:
                    error('Unknown system!')
                modules.popen(cmd, shell=True, cwd=os.getcwd())
            else:
                linux = "mkdir " + self.dstdir + "/platform"
                cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
                if not cmd:
                    error('Unknown system!')
                modules.popen(cmd, shell=True, cwd=os.getcwd())
                linux = "cp -rf " + self.srcdir + "/platform/arch " + self.dstdir + "/platform/"
                cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
                if not cmd:
                    error('Unknown system!')
                modules.popen(cmd, shell=True, cwd=os.getcwd())
                linux = "mkdir " + self.dstdir + "/platform/mcu"
                cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
                if not cmd:
                    error('Unknown system!')
                modules.popen(cmd, shell=True, cwd=os.getcwd())
                linux = "cp -rf " + self.srcdir + "/platform/mcu/linux " + self.dstdir + "/platform/mcu" 
                cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
                if not cmd:
                    error('Unknown system!')
                modules.popen(cmd, shell=True, cwd=os.getcwd())
                linux = "cp -rf " + self.srcdir + "/platform/mcu/include " + self.dstdir + "/platform/mcu" 
                cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
                if not cmd:
                    error('Unknown system!')
                modules.popen(cmd, shell=True, cwd=os.getcwd())

            if os.path.exists("./board"):
                linux = "cp -rf " + "./board "+ self.dstdir
                cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
                if not cmd:
                    error('Unknown system!')
                modules.popen(cmd, shell=True, cwd=os.getcwd())
            else:
                linux = "mkdir board"
                cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
                if not cmd:
                    error('Unknown system!')
                modules.popen(cmd, shell=True, cwd=os.getcwd())
                linux = "cp -rf " + self.srcdir + "/board/linuxhost/* "+ "./board/"
                cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
                if not cmd:
                    error('Unknown system!')
                modules.popen(cmd, shell=True, cwd=os.getcwd())

            if os.path.exists("./example"):
                linux = "cp -rf " + "./example "+ self.dstdir
            else:
                linux = "cp -rf " + self.srcdir + "/example "+ self.dstdir
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())
        elif self.base == "telink":
            src = self.srcdir + "/alinkconfig.db"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/bootloader"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/device"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/example/nano"
            linux = "cp -rf " + src + " ./"
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/example/*"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            dst = self.srcdir + "/example"
            linux = "cp -rf " + "./nano " + dst
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/script"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/tags"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/doc"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/framework"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/security"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/test"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            # kernel folder
            src = self.srcdir + "/kernel/rhino"
            linux = "cp -rf " + src + " ./"
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/kernel/vcall"
            linux = "cp -rf " + src + " ./"
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/kernel/vfs"
            linux = "cp -rf " + src + " ./"
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/kernel/yloop"
            linux = "cp -rf " + src + " ./"
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/kernel/*"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/kernel"
            linux = "cp -rf ./rhino " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/kernel"
            linux = "cp -rf ./vcall " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/kernel"
            linux = "cp -rf ./vfs " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/kernel"
            linux = "cp -rf ./yloop " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.dstdir + "/board/linuxhost"
            linux = "cp -rf " + src + " ./"
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/board/*"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/board"
            linux = "cp -rf ./linuxhost " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.dstdir + "/platform/mcu/linux"
            linux = "cp -rf " + src + " ./"
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/platform/mcu/include"
            linux = "cp -rf " + src + " ./"
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/platform/mcu/*"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/platform/mcu"
            linux = "cp -rf ./linux " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/platform/mcu/"
            linux = "cp -rf ./include " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.dstdir + "/build"
            linux = "cp -rf " + src + " ./"
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/build"
            linux = "rm -rf " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir
            linux = "cp -rf ./build " + src
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            dst = self.dstdir + "/*"
            linux = "rm -rf " + dst
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            dst = self.dstdir
            src = self.srcdir + "/*"
            linux = "cp -rf " + src + " " + dst
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

        else:
            dst = self.dstdir + "/*"
            linux = "rm -rf " + dst
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            dst = self.dstdir + "/.gitignore"
            linux = "rm -rf " + dst
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            dst = self.dstdir + "/.vscode"
            linux = "rm -rf " + dst
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

            src = self.srcdir + "/*"
            dst = self.dstdir
            linux = "cp -rf " + src + " " + dst
            cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
            if not cmd:
                error('Unknown system!')
            modules.popen(cmd, shell=True, cwd=os.getcwd())

    def make_folder(self, module):
        dst = ""
        if self.base == "mxchip" or self.base == "github":
            if module == "mesh":
                dst = self.dstdir + "/kernel/protocols/mesh/lib/mk3060"
            elif module == "ywss":
                dst = self.dstdir + "/framework/ywss/lib/mk3060"
            elif module == "rhino":
                dst = self.dstdir + "/kernel/rhino/lib/mk3060"
            elif module == "wsf":
                dst = self.dstdir + "/framework/connectivity/wsf/lib/mk3060"
            elif module == "msdp":
                if self.srcbase == "1.0.1":
                    base_dstdir = self.dstdir + "/framework/protocol/alink/msdp/"
                else:
                    base_dstdir = self.dstdir + "/framework/gateway/msdp/"
                dst = base_dstdir + "lib/mk3060"
            elif module == "devmgr":
                if self.srcbase == "1.0.1":
                    base_dstdir = self.dstdir + "/framework/protocol/alink/devmgr/"
                else:
                    base_srcdir = self.dstdir + "/framework/gateway/devmgr/"
                dst = base_dstdir + "lib/mk3060"
            elif module == "gateway":
                dst = self.dstdir + "/framework/gateway/lib/mk3060"
            else:
                return 0
        else:
            return 0

        linux = "mkdir " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
            return 1
        modules.popen(cmd, shell=True, cwd=os.getcwd())
        return 0

    def make_lib(self, module):
        linux = ""
        mac = ""
        win = ""
        if self.base == "mxchip" or self.base == "github":
            if module == "mesh":
                linux = "aos makelib -r ARM968E-S kernel/protocols/mesh"
            elif module == "ywss":
                linux = "aos makelib -r ARM968E-S framework/ywss"
            elif module == "rhino":
                linux = "aos makelib -r ARM968E-S kernel/rhino"
            elif module == "wsf":
                linux = "aos makelib -r ARM968E-S framework/connectivity/wsf"
            elif module == "msdp":
                linux = "aos makelib -r ARM968E-S framework/gateway/msdp"
            elif module == "devmgr":
                linux = "aos makelib -r ARM968E-S framework/gateway/devmgr"
            elif module == "gateway":
                linux = "aos makelib -r ARM968E-S framework/gateway"
            else:
                return 0
        else:
            return 0

        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
            return 1
        modules.popen(cmd, shell=True, cwd=os.getcwd())
        return 0

    def copy_lib(self, module):
        linux = ""
        mac = ""
        win = ""
        if self.base == "mxchip" or self.base == "github":
            if module == "mesh":
                src = self.srcdir + "/mesh.ARM968E-S.mk3060.GCC.release.a"
                dst = self.dstdir + "/kernel/protocols/mesh/lib/mk3060/libmesh.a"
            elif module == "ywss":
                src = self.srcdir + "/ywss.ARM968E-S.mk3060.GCC.release.a"
                dst = self.dstdir + "/framework/ywss/lib/mk3060/libywss.a"
            elif module == "rhino":
                src = self.srcdir + "/rhino.ARM968E-S.mk3060.GCC.release.a"
                dst = self.dstdir + "/kernel/rhino/lib/mk3060/librhino.a"
            elif module == "wsf":
                src = self.srcdir + "/wsf.ARM968E-S.mk3060.GCC.release.a"
                dst = self.dstdir + "/framework/connectivity/wsf/lib/mk3060/libwsf.a"
            elif module == "msdp":
                src = self.srcdir + "/msdp.ARM968E-S.mk3060.GCC.release.a"
                dst = self.dstdir + "/framework/gateway/msdp/lib/mk3060/libmsdp.a"
            elif module == "devmgr":
                src = self.srcdir + "/devmgr.ARM968E-S.mk3060.GCC.release.a"
                dst = self.dstdir + "/framework/gateway/devmgr/lib/mk3060/libdevmgr.a"
            elif module == "gateway":
                src = self.srcdir + "/gateway.ARM968E-S.mk3060.GCC.release.a"
                dst = self.dstdir + "/framework/gateway/lib/mk3060/libgateway.a"
            else:
                return 0
        else:
            return 0

        linux = "cp -f " + src + " " + dst
        cmd = mac if sys.platform == 'darwin' else (linux if sys.platform == 'linux2' else (win if sys.platform == 'win32' else None))
        if not cmd:
            error('Unknown system!')
            return 1
        modules.popen(cmd, shell=True, cwd=os.getcwd())
        return 0

