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

filterOutType = ['h','c','cpp','s','S','ld']

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

def _disableSyscall(sn_path):
    if os.path.exists(sn_path):
        fsn = open(sn_path, "r+")              # read from syscall_num
        sysdata = fsn.readlines()
        fsn.seek(0)
        for line in sysdata:
            u = line[-(len(line) - line.find((" "))):-1]
            logging.debug(u)
            line = r"%s %s" %(0, u.strip()) + "\n"
            fsn.write(line)
        fsn.close()

    return

def _writeSyscallHeader(cr_path, sh_path, sn_path):
    fcr = open(cr_path, 'r')               # read copyright
    copyright = fcr.read()
    fcr.close()

    fsh = open(sh_path, "w+")              # creat syscall_tbl.h
    fsh.seek(0, 0)
    fsh.write(copyright)
    fsh.write("\n")

    if os.path.exists(sn_path):
        fsn = open(sn_path, "r+")              # read from syscall_num
    else:
        fsn = open(sn_path, "w+")              # read from syscall_num
    sysdata = fsn.readlines()
    sysdata_num = len(sysdata)
    fsn.seek(0)
    global symbol_list
    find = 0
    for symbol in symbol_list:                          # write to syscall_num
        fsn.seek(0)
        for line in sysdata:
            if(re.findall(r"\d+\s\d+\s" + symbol[1] + r"\s\".*?\"\s\".*?\"\n", line, re.M | re.S)):
                serial_num = line.strip().split(" ", line.strip().count(" "))[1]
                line = r"%s %s %s %s %s" %(1, serial_num, symbol[1], "\"" + symbol[0].replace("\n", "") + "\"", symbol[2].replace("\n", "")) + "\n"
                find = 1
                logging.debug(line)
                fsn.write(line)
            fsn.seek(len(line), 1)
        if find == 0:
            sysdata_num += 1
            line = r"%s %s %s %s %s" %(1, sysdata_num, symbol[1], "\"" + symbol[0].replace("\n", "") + "\"", symbol[2].replace("\n", "")) + "\n"
            fsn.seek(0, 2)
            fsn.write(line)
        find = 0

	fsn.flush()
    fsn.seek(0, 0)
    fsnContent = fsn.read()
    newsymbols = re.findall(r"(\d+)\s(\d+)\s(.*?)\s\"(.*?)\"\s\"(.*?)\"\n", fsnContent, re.M | re.S)
    logging.debug(newsymbols)
    global syscall_num
    syscall_num = len(newsymbols)
    for symbol in newsymbols:                     # according to syscall_num to implementation syscall_tbl.h
        logging.debug(symbol[0])
        if symbol[0] == str(1):
            fsh.write("#if (" + symbol[3] + ")\n")
            strdef = "#define SYS_" + symbol[2].upper() + " " + symbol[1] + "\n"
            strsysc = "SYSCALL(SYS_" + symbol[2].upper() + ", " + symbol[2] + ")"
            fsh.write(strdef + strsysc + "\n")
            fsh.write("#endif" + "\n\n")

    fsn.close()
    fsh.close()

    return

def _writeSyscallUapi(sc_path, sn_path, ui_path):
    fui = open(ui_path, 'r')               # read usyscall include
    usys_incl = fui.read()
    fui.close()

    fsc = open(sc_path, "w+")              # creat syscall_uapi.c
    fsc.seek(0, 0)
    fsc.write(usys_incl)
    fsc.write("\n")

    fsn = open(sn_path, 'r')               # read usyscall data
    fsnContent = fsn.read()
    fsn.close()

    newsymbols = re.findall(r"(\d+)\s(\d+)\s(.*?)\s\"(.*?)\"\s\"(.*?)\"\n", fsnContent, re.M | re.S)
    logging.debug(newsymbols)
    for symbol in newsymbols:                     # according to syscall_num to implementation syscall_tbl.h
        logging.debug(symbol)
        if symbol[0] == str(1):
            fsc.write("#if (" + symbol[3] + ")\n" + symbol[4] + "\n" + "{\n" + "    ")
            elements = re.findall(r"(.*?)" + symbol[2] + r"\((.*?)\)$", symbol[4], re.M | re.S)
            logging.debug(elements)
            needreturn = 0
            for element in elements:
                logging.debug(element[1])
                args = element[1].split(',')
                i = 0
                for arg in args:
                    while args[i].count("(") != args[i].count(")"):
                        args[i] = args[i] + "," + args[i+1]
                        args.pop(i+1)
                    i += 1
                arg_nu = len(args)

                if arg_nu == 1:
                    if args[0].strip() == r"void" or args[0].strip() == r"":
                        arg_nu = 0
                logging.debug(args)
                logging.debug(arg_nu)

            fsc.write(r"if (SYSCALL_TBL[" + "SYS_" + symbol[2].upper() + "] != NULL) {\n" + "        ")
            if element[0].strip() != r"void":
                fsc.write("return ")
                needreturn = 1
            fsc.write("SYS_CALL" + str(arg_nu) + "(SYS_" + symbol[2].upper() + ", " + element[0].strip())
            i = 0
            if arg_nu == 0:
                fsc.write(r");")
            else:
                for arg in args:
                    if "(" in arg.strip():
                        u = arg.strip().split("(*")
                        u1 = u[0] + "(*)(" + u[1].split(")(")[1]
                        u2 = u[1].split(")(")[0]
                    elif "*" in arg.strip():
                        u1 = arg[0:(arg.index("*") + arg.count("*"))]
                        u2 = arg[-(len(arg) - len(u1)):]
                    else:
                        u = arg.strip().split(" ", arg.strip().count(" "))
                        u2 = u[arg.strip().count(" ")]
                        u1 = arg[0:(len(arg) - len(u2))]

                    logging.debug(u1)
                    logging.debug(u2)
                    logging.debug(len(args))
                    i += 1
                    if u1 != "":
                        fsc.write(", " + u1.strip())
                    if u2 != "":
                        fsc.write(", " + u2.strip())
                        if i == len(args):
                            fsc.write(r");")

            fsc.write("\n    } else {\n" + "        ")
            fsc.write("LOGE(\"BINS\", \"%s is NULL in SYSCALL_TBL\", __func__);\n")
            if needreturn == 1:
                fsc.write("        return;\n")
            fsc.write("    }\n}\n#endif" + "\n\n")

    fsc.close()

    return

def _writeSyscallMk(sm_path):
    fsh = open(sm_path, "w+")              # creat usyscall.mk
    fsh.seek(0, 0)
    fsh.write(r"NAME := usyscall" + "\n\n")
    fsh.write(r"$(NAME)_INCLUDES := ./ ../../../framework/usyscall" + "\n\n")
    fsh.write(r"$(NAME)_CFLAGS += -Wall -Werror" + "\n\n")
    fsh.write(r"$(NAME)_SOURCES := syscall_uapi.c" + "\n\n")
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

def _removeSyscallData(sn_path):
    if os.path.exists(sn_path):
        logging.debug(sn_path)
        os.remove(sn_path)                     # remove syscall_num

    return

def main():
    syscall_path = sys.argv[1]
    logging.info(sys.argv[1])

    search_string = r"EXPORT_SYMBOL_K\((.*?)\,\s*?[\\|\s]\s*?(\S*?)\,\s*?[\\|\s]\s*?(\".*?\")\)$"
    copyright_path = r"./build/copyright"
    syscall_tblc_path = r"./kernel/syscall/syscall_tbl.c"
    syscall_tbl_path = syscall_path + r"/syscall_tbl.h"
    syscall_uapi_path = syscall_path + r"/syscall_uapi.c"
    syscall_mk_path = syscall_path + r"/usyscall.mk"
    syscall_data_path = r"./build/scripts/syscall_data"
    usyscall_incl_path = r"./framework/usyscall/syscall_uapi_include.h"
    global symbol_list
    global num

    starttime = time.time()

    # Search for each directory, find the symbol
    for rootdir in rootdirs:
        search_symbols(rootdir, search_string)

    # Remove duplicate element & Element sorting
    symbol_list=sorted(set(symbol_list),key=symbol_list.index)
    symbol_list.sort()

    # set syscall serial num to 0
    _disableSyscall(syscall_data_path)

    logging.info("======================================")
    logging.info(" new symbol:")
    # Creat and write to syscall_tbl.h
    _writeSyscallHeader(copyright_path, syscall_tbl_path, syscall_data_path)
    logging.info("======================================")

    # Creat and write to syscall_uapi.c
    _writeSyscallUapi(syscall_uapi_path, syscall_data_path, usyscall_incl_path)

    # Creat and write to usyscall.mk
    _writeSyscallMk(syscall_mk_path)

    #modify SYSCALL_MAX
    _modifySyscallMax(syscall_tblc_path)
    endtime = time.time()

    print "======================================"
    print (" create syscall file:")
    print (" total: %s symbol find." % len(symbol_list))
    print (" total: %s file find." % num)
    print (" total time: %s s." % (endtime - starttime))
    print "======================================"

if __name__ == "__main__":
    main()

