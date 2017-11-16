#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import sys,os,re
import time,logging

rootdirs = ['./board',         \
            './bootloader',    \
            './devices',       \
            './example',       \
            './framework',     \
            './include',       \
            './kernel',        \
            './platform',      \
            './security',      \
            './test',          \
            './tools',         \
            './utility']

filterType = ['gif','png','bmp','jpg','jpeg','rar','zip',
            'ico','apk','ipa','doc','docx','xls','jar',
            'xlsx','ppt','pptx','pdf','gz','pyc','class']

filterOutType = ['h','c','cpp','s','S','ld','i']

num = 0
syscall_num = 0
symbol_list = []

# DEBUG < INFO < WARNING < ERROR < CRITICAL
logging.basicConfig(level=logging.WARNING)


def search_symbols(path=None, cont=None):
    if not path or not cont:
        print('path or searchString is empty')
        return
    _loopFolder(path, cont)

    return

def _loopFolder(path, cont):
    arr = path.split('/')
    if not arr[-1].startswith('.'):            #Do not check hidden folders
        if os.path.isdir(path):
            folderList = os.listdir(path)
            logging.debug(folderList)
            for x in folderList:
                _loopFolder(path + "/" + x, cont)
        elif os.path.isfile(path):
            _verifyContent(path, cont)

    return

def _verifyContent(path, cont):
    if path.split('.')[-1].lower() in filterType:
        return
    global num
    global symbol_list
    try:
        fh = open(path, 'r+')
        fhContent = fh.read()
        fh.close()
        symbols = re.findall(cont, fhContent, re.M | re.S)
        if symbols:
                logging.debug(symbols)
	        symbol_list.extend(symbols)
	        num += 1
                logging.debug("%s" % (path))
    except:
        print "File '" + path + "'can't be read"

    return

def _disableSyscall(sd_path):
    if os.path.exists(sd_path):
        fsn = open(sd_path, "r+")              # read from syscall_num
        sysdata = fsn.readlines()
        fsn.seek(0)
        for line in sysdata:
            u = line[-(len(line) - line.find((" "))):-1]
            logging.debug(u)
            line = r"%s %s" %(0, u.strip()) + "\n"
            fsn.write(line)
        fsn.close()

    return

def _writeSyscallHeader(cr_path, sh_path, sd_path, sn_path, stype):
    fcr = open(cr_path, 'r')               # read copyright
    copyright = fcr.read()
    fcr.close()

    fsh = open(sh_path, "w+")              # creat syscall_tbl.h
    fsh.seek(0, 0)
    fsh.write(copyright)
    fsh.write("#define SYSCALL_MAX 0" + "\n")
    fsh.write("#define SYSCALL(nr, func) [nr] = func," + "\n\n")
    if stype == "kernel":
        fsh.write("const void *syscall_ktbl[] __attribute__ ((section(\".syscall_ktbl\"))) = {" + "\n")
    elif stype == "framework":
        fsh.write("const void *syscall_ftbl[] __attribute__ ((section(\".syscall_ftbl\"))) = {" + "\n")

    fsh.write("[0 ... SYSCALL_MAX - 1] = (void *)NULL," + "\n\n")

    fsn = open(sn_path, "w+")              # creat syscall_num.h
    fsn.seek(0, 0)
    fsn.write(copyright)


    if os.path.exists(sd_path):
        fsd = open(sd_path, "r+")              # read from syscall_data
    else:
        fsd = open(sd_path, "w+")              # read from syscall_data
    sysdata = fsd.readlines()
    sysdata_num = len(sysdata)
    global symbol_list
    find = 0
    for symbol in symbol_list:                          # write to syscall_data
        for line in sysdata:
            if(re.findall(r"\d+\s\d+\s" + symbol[1] + r"\s\".*?\"\s\".*?\"\n", line, re.M | re.S)):
                serial_num = line.strip().split(" ", line.strip().count(" "))[1]
                newline = r"%s %s %s %s %s" %(1, serial_num, symbol[1], "\"" + symbol[0] + "\"", "\"" + symbol[2] + "\"") + "\n"
                find = 1
                logging.debug(newline)
                sysdata[sysdata.index(line)] = newline
                break
        if find == 0:
            line = r"%s %s %s %s %s" %(1, sysdata_num, symbol[1], "\"" + symbol[0] + "\"", "\"" + symbol[2] + "\"") + "\n"
            sysdata.append(line)
            sysdata_num += 1
        find = 0

    fsd.truncate(0)
    fsd.seek(0, 0)
    fsd.writelines(sysdata)
    fsd.flush()
    fsd.seek(0, 0)
    fsnContent = fsd.read()
    newsymbols = re.findall(r"(\d+)\s(\d+)\s(.*?)\s\"(.*?)\"\s\"(.*?)\"\n", fsnContent, re.M | re.S)
    logging.debug(newsymbols)
    global syscall_num
    syscall_num = len(newsymbols)
    for symbol in newsymbols:                     # according to syscall_data to implementation syscall_tbl.h
        logging.debug(symbol[0])
        if symbol[0] == str(1):
            # create syscall_ktbl.h
            strdef = "#define SYS_" + symbol[2].upper() + " " + symbol[1] + "\n"
            strsysc = "SYSCALL(SYS_" + symbol[2].upper() + ", " + symbol[2] + ")"
            fsh.write(strdef + strsysc + "\n")
            fsh.write("\n")

            # create syscall_knum.h
            fsn.write(strdef)
            fsn.write("\n")
    fsh.write("};" + "\n")

    fsn.close()
    fsd.close()
    fsh.close()

    return

def _writeSyscallUapi(sc_path, sd_path, ui_path, stype):
    fui = open(ui_path, 'r')               # read xsyscall_include
    usys_incl = fui.read()
    fui.close()

    fsc = open(sc_path, "w+")              # creat xsyscall_tbl.c
    fsc.seek(0, 0)
    fsc.write(usys_incl)
    fsc.write("\n")

    fsd = open(sd_path, 'r')               # read xsyscall_data
    fsnContent = fsd.read()
    fsd.close()

    newsymbols = re.findall(r"(\d+)\s(\d+)\s(.*?)\s\"(.*?)\"\s\"(.*?)\"\n", fsnContent, re.M | re.S)
    logging.debug(newsymbols)
    for symbol in newsymbols:                     # according to syscall_data to implementation syscall_api.c
        logging.debug(symbol)
        if symbol[0] == str(1):
            fsc.write(symbol[3].strip() + " " + symbol[2].strip() + "(")
            args_num = symbol[4].strip().count(",")
            args = symbol[4].strip().split(",")
            i = 0
            snum = 0
            conti = 0
            for arg in args:
                if conti == 1:
                    if ")" in arg.strip():
                        conti = 0
                    fsc.write(arg.strip())

                    if i != args_num:
                        fsc.write(", ")
                    i += 1
                else:
                    if arg.strip() == r"void":
                        fsc.write(arg.strip())
                    else:
                        if "(*" in arg.strip():
                            substr = arg.strip().split("(*")
                            fsc.write(substr[0] + "(*a" + str(snum) + substr[1])
                            if (substr[1].count(")") % 2) != 0:
                                conti = 1
                            snum += 1
                        else:
                            if arg.strip() == r"...":
                                fsc.write(arg.strip())
                            else:
                                fsc.write(arg.strip() + " a" + str(snum))
                                snum += 1
    
                        if i != args_num:
                            fsc.write(", ")
                        i += 1

            fsc.write(")" + "\n" + "{\n")
            if r"..." in symbol[4].strip():
                fsc.write("    va_list args;\n    char message[128];\n    int ret;\n\n    memset(message, 0, 128);\n\n")
                fsc.write("    va_start(args, a0);\n    ret = vsnprintf(message, 128, a0, args);\n    va_end(args);\n\n")
            fsc.write(r"    if (SYSCALL_TBL[" + "SYS_" + symbol[2].upper() + "] != NULL) {\n" + "        ")
            needreturn = 0
            if symbol[3].strip() != r"void":
                fsc.write("return ")
                needreturn = 1

            fsc.write("SYS_CALL" + str(snum) + "(SYS_" + symbol[2].upper() + ", " + symbol[3].strip())
            snum = 0
            conti = 0
            for arg in args:
                if arg.strip() != r"void":
                    if conti == 1:
                        if ")" in arg.strip():
                            conti = 0
                            fsc.write(", " + arg.strip() + ", " + "a" + str(snum))
                            snum += 1
                        else:
                            fsc.write(", " + arg.strip())
                    else:
                        if "(*" in arg.strip():
                            if (arg.strip().count(")") % 2) == 0:
                                fsc.write(", " + arg.strip() + ", " + "a" + str(snum))
                                snum += 1
                            else:
                                fsc.write(", " + arg.strip())
                                conti = 1
                        else:
                            if arg.strip() != r"...":
                                if r"..." in symbol[4].strip():
                                    fsc.write(", " + arg.strip() + ", " + "message")
                                else:
                                    fsc.write(", " + arg.strip() + ", " + "a" + str(snum))
                            snum += 1

            fsc.write(r");")

            fsc.write("\n    } else {\n" + "        ")
            fsc.write("LOGE(\"BINS\", \"%s is NULL in SYSCALL_TBL\", __func__);\n")
            if needreturn == 1:
                fsc.write("        return;\n")
            fsc.write("    }\n}\n\n")

    fsc.close()

    return

def _writeKsyscallMk(sm_path):
    fsh = open(sm_path, "w+")              # creat fsyscall.mk
    fsh.seek(0, 0)
    fsh.write(r"NAME := syscall_kapi" + "\n\n")
    fsh.write(r"$(NAME)_TYPE := app&framework" + "\n\n")
    fsh.write(r"$(NAME)_INCLUDES := ./ ../../../framework/fsyscall/syscall_kapi" + "\n\n")
    fsh.write(r"$(NAME)_CFLAGS += -Wall -Werror" + "\n\n")
    fsh.write(r"$(NAME)_SOURCES := syscall_kapi.c" + "\n\n")
    fsh.write(r"GLOBAL_DEFINES += AOS_BINS" + "\n")

    fsh.close()

    return

def _writeFsyscallMk(sm_path):
    fsh = open(sm_path, "w+")              # creat fsyscall.mk
    fsh.seek(0, 0)
    fsh.write(r"NAME := syscall_fapi" + "\n\n")
    fsh.write(r"$(NAME)_TYPE := app" + "\n\n")
    fsh.write(r"$(NAME)_INCLUDES := ./ ../../../app/usyscall/syscall_fapi" + "\n\n")
    fsh.write(r"$(NAME)_CFLAGS += -Wall -Werror" + "\n\n")
    fsh.write(r"$(NAME)_SOURCES := syscall_fapi.c" + "\n\n")
    fsh.write(r"GLOBAL_DEFINES += AOS_BINS" + "\n")

    fsh.close()

    return

def _modifySyscallMax(sc_path):
    global syscall_num
    fcr = open(sc_path, 'r+')               # read syscall_tbl.c
    tblc = fcr.readlines()
    fcr.seek(0)
    logging.debug(syscall_num)
    for line in tblc:
        if(line.find(r"#define SYSCALL_MAX") == 0):
            line = r"#define SYSCALL_MAX %s" % (syscall_num + 1) + "\n"   
        fcr.write(line)
    fcr.close()

    return

def _removeSyscallData(sd_path):
    if os.path.exists(sd_path):
        logging.debug(sd_path)
        os.remove(sd_path)                     # remove syscall_num

    return

def _preCreateSyscall(path):
    fpath = open(path, "w+")
    fpath.close()
    return

def main():
    syscall_path = sys.argv[1]
    logging.info(sys.argv[1])

    # stage for create syscall, pre-create/create
    syscall_stage = sys.argv[2]
    logging.info(sys.argv[2])

    search_string = r"AOS_EXPORT\((.*?)\s*?\,\s*?[\\|\s]\s*?(\S*?)\s*?\,\s*?(.*?)\)$"
    copyright_path = r"./build/copyright"
    ksearch_rootdirs = syscall_path + r"/precompile/kernel"
    ksearch_rootdirs_additional = r"kernel/ksyscall"
    fsearch_rootdirs = syscall_path + r"/precompile/framework"
    fsearch_rootdirs_additional = r"framework/fsyscall"

    ksyscall_kapi_path = syscall_path + r"/syscall/syscall_kapi/syscall_kapi.c"
    ksyscall_tbl_path = syscall_path + r"/syscall/syscall_kapi/syscall_ktbl.h"
    ksyscall_num_path = syscall_path + r"/syscall/syscall_kapi/syscall_knum.h"
    ksyscall_mk_path = syscall_path + r"/syscall/syscall_kapi/syscall_kapi.mk"
    ksyscall_data_path = r"./build/scripts/syscall_kdata"
    ksyscall_incl_path = r"./framework/fsyscall/syscall_kapi/syscall_kapi.h"

    fsyscall_fapi_path = syscall_path + r"/syscall/syscall_fapi/syscall_fapi.c"
    fsyscall_tbl_path = syscall_path + r"/syscall/syscall_fapi/syscall_ftbl.h"
    fsyscall_num_path = syscall_path + r"/syscall/syscall_fapi/syscall_fnum.h"
    fsyscall_mk_path = syscall_path + r"/syscall/syscall_fapi/syscall_fapi.mk"
    fsyscall_data_path = r"./build/scripts/syscall_fdata"
    fsyscall_incl_path = r"./app/usyscall/syscall_fapi.h"

    if syscall_stage == r"pre-create":
        # pre-create ksyscall
        _preCreateSyscall(ksyscall_kapi_path)
        _preCreateSyscall(ksyscall_tbl_path)
        _preCreateSyscall(ksyscall_num_path)
        _writeKsyscallMk(ksyscall_mk_path)
        # pre-create fsyscall
        _preCreateSyscall(fsyscall_fapi_path)
        _preCreateSyscall(fsyscall_tbl_path)
        _preCreateSyscall(fsyscall_num_path)
        _writeFsyscallMk(fsyscall_mk_path)
    elif syscall_stage == r"create":
        global symbol_list
        global num
        global syscall_num
    
        starttime = time.time()
    
        ######################## ksyscall ##############################
    
        # Search for each directory, find the symbol
        search_symbols(ksearch_rootdirs, search_string)
        #search_symbols(ksearch_rootdirs_additional, search_string)
    
        # Remove duplicate element & Element sorting
        symbol_list = sorted(set(symbol_list),key=symbol_list.index)
        symbol_list = sorted(symbol_list, key=lambda fun: fun[1].lower())
    
        # set syscall serial num to 0
        _disableSyscall(ksyscall_data_path)
    
        stype = "kernel"
        logging.info("======================================")
        logging.info(" ksyscall new symbol:")
        # Creat and write to syscall_ktbl.h syscall_knum.h syscall_kdata
        _writeSyscallHeader(copyright_path, ksyscall_tbl_path, ksyscall_data_path, ksyscall_num_path, stype)
        logging.info("======================================")
    
        # Creat and write to syscall_kapi.c
        _writeSyscallUapi(ksyscall_kapi_path, ksyscall_data_path, ksyscall_incl_path, stype)
    
        # Creat and write to ksyscall.mk
        #_writeKsyscallMk(ksyscall_mk_path)
    
        #modify SYSCALL_MAX
        _modifySyscallMax(ksyscall_tbl_path)
    
        print "======================================"
        print (" create ksyscall file:")
        print (" total: %s ksymbol find." % len(symbol_list))
        print (" total: %s file find." % num)
        print "--------------------------------------"
    
        ######################## fsyscall ##############################
    
        # reset global variable
        symbol_list = []
        num = 0
        syscall_num = 0
    
        # Search for each directory, find the symbol
        search_symbols(fsearch_rootdirs, search_string)
        #search_symbols(fsearch_rootdirs_additional, search_string)

    
        # Remove duplicate element & Element sorting
        symbol_list = sorted(set(symbol_list),key=symbol_list.index)
        symbol_list = sorted(symbol_list, key=lambda fun: fun[1].lower())
    
        # set syscall serial num to 0
        _disableSyscall(fsyscall_data_path)
    
        stype = "framework"
        logging.info("======================================")
        logging.info(" fsyscall new symbol:")
        # Creat and write to syscall_ftbl.h syscall_fnum.h syscall_fdata
        _writeSyscallHeader(copyright_path, fsyscall_tbl_path, fsyscall_data_path, fsyscall_num_path, stype)
        logging.info("======================================")
    
        # Creat and write to syscall_fapi.c
        _writeSyscallUapi(fsyscall_fapi_path, fsyscall_data_path, fsyscall_incl_path, stype)
    
        # Creat and write to fsyscall.mk
        #_writeFsyscallMk(fsyscall_mk_path)
    
        #modify SYSCALL_MAX
        _modifySyscallMax(fsyscall_tbl_path)
    
        endtime = time.time()
    
        print (" create fsyscall file:")
        print (" total: %s fsymbol find." % len(symbol_list))
        print (" total: %s file find." % num)
        print "--------------------------------------"
        print (" total time: %s s." % (endtime - starttime))
        print "======================================"

if __name__ == "__main__":
    main()

