import sys
import errno

#########################################
# add supported chip vendors branch here
#########################################

class vendorslib:
    def __init__(self, base):
        self.base = base

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
        else:
            ret = 1
        return ret
