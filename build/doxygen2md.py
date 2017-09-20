#!/usr/bin/env python

import os
import re

def annotation_get(f):

    fd = open(f, 'r')
    code = fd.read()
    fd.close()

    return re.findall(r'/\*\*.*?\);', code, re.DOTALL)

def annotation_analyse(num, annotation, templet):

    # deal with function
    func = re.findall(r'\*/.*?\);', annotation, re.DOTALL)
    func_name = (func[0].partition(" "))[2].partition("(")[0]

    function = (re.findall(r'\*\/\n(.*?);', func[0], re.DOTALL))[0]

    func_desc = ((re.findall(r'(/\*\*\n \* )(.*?)(\n \*\n \* @)', annotation, re.DOTALL))[0][1]).replace(" *", " ")

    templet_head = (templet.partition("  PARAM_DESC"))[0]
    templet_mid  = ''
    templet_tail = (templet.partition("  PARAM_DESC"))[2]

    func_params = (re.findall(r'(@param.*?)\n \*\n \* @', annotation, re.DOTALL))[0].split('\n * ')
    for param in func_params:
        params = "  | " + (param.strip("@param")).replace("  ", " | ", 2) + " |"
        templet_mid = templet_mid + params + "\n"

    templet = templet_head + templet_mid + templet_tail

    func_return = (re.findall(r'(@return  )(.*?)(\n \*/)', annotation, re.DOTALL))[0][1]

    templet = templet.replace("FUNC_NUM", str(num))
    templet = templet.replace("FUNC_NAME", func_name)
    templet = templet.replace("FUNCTION", function)
    templet = templet.replace("FUNC_DESC", func_desc)
    templet = templet.replace("RETURN_DESC", func_return)

    print templet

    return templet

def doxygen2md(f, templet):

    print f

    fd = open("./out/" + os.path.basename(f) + ".md", 'w')

    num = 0
    for annotation in annotation_get(f):
        num += 1
        md = annotation_analyse(num, annotation, templet)
        fd.write(md)

    fd.close()

def main():
    fs = []

    for root, dirs, files in os.walk('./include'):
        for f in files:
            if (os.path.splitext(f)[1] == ".h"):
                fs.append(os.path.join(root, f))

    fd = open("./build/MD.templet", 'r')
    templet = fd.read()
    fd.close()

    for f in fs:
        doxygen2md(f, templet)

if __name__ == '__main__':
    main()

