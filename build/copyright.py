#!/usr/bin/env python

import os
import re

rootdirs = ['./bootloader',            \
            './framework',             \
            './include',               \
            './example',               \
            './devices',               \
            './kernel/hal',            \
            './kernel/modules/kv',     \
            './kernel/protocols/mesh', \
            './kernel/rhino',          \
            './kernel/syscall',        \
            './kernel/vcall/yos',      \
            './kernel/vfs',            \
            './kernel/yloop']

def list_files(dir_path):
    file_list = [];

    for root, dirs, files in os.walk(dir_path):
        for f in files:
            if (os.path.splitext(f)[1] == ".h" or os.path.splitext(f)[1] == ".c"):
                file_list.append(os.path.join(root, f))
    return file_list

def insert_copyright(f, regex, copyright):
    fo = open(f, 'r+')
    string = fo.read()
    if not re.search(regex, string, re.M):
        fo.seek(0, 0)
        fo.write(copyright)
        fo.write(string)

    fo.close()

def main():
    regex = r'Copyright'

    fo = open("./build/copyright", 'r')
    copyright = fo.read()
    fo.close()

    for rootdir in rootdirs:
        file_list = list_files(rootdir)
        for f in file_list:
            insert_copyright(f, regex, copyright)

if __name__ == '__main__':
    main()

