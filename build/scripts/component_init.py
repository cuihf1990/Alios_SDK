import sys, os, json, glob, re
from itertools import chain

component_init_src = "component_init.c"

def process_config_mk(config_mk_str, config_mk_file):
    global component_init_src

    config_mk_replace = False
    config_mk_lines = config_mk_str.splitlines()

    for i, line in enumerate(config_mk_lines):
        if line.find("auto_component_SOURCES") != -1: #auto component line
            if line.find(component_init_src) == -1: #add source file to config.mk
                replace_str = line + " " + component_init_src
                config_mk_lines[i] = replace_str                
                config_mk_replace = True
    
    if config_mk_replace:
        #auto component change config.mk.   
        with open(config_mk_file, "w") as config_mk:
            for line in config_mk_lines:
                config_mk.write("%s\n" % line)


def process_component_init(config_mk_str, init_dir):
    global component_init_src

    init_codes = "/*\n * warning: component init code, don't change!!!\n */\n\n#include <aos/aos.h>\n\nvoid aos_components_init(void) {\n"

    # find all head file through config.mk 
    for root, dirs, files in  chain.from_iterable(os.walk(path.strip()) for path in \
            re.findall("AOS_SDK_INCLUDES\s+\+\=\s+(.+\n)", config_mk_str)[0].split("-I")[1:]):
        for include in [include for include in files if include.endswith(".h")]:
            with open(os.path.join(root, include), "r") as head:
                code = head.read()
                # find AOS_COMPONENT_INIT macro function
                for init in re.findall("AOS_COMPONENT_INIT\s*\((.+\)\s*;)", code):
                    init_code = init.replace(',', '(', 1)

                    if init == init_code:#init function no argument
                        init_code = init[:len(init)-2] +'(' + init[len(init)-2:]

                    init_codes += '    ' + init_code + '\n'  
    
    init_codes += "}"
    # write component init code.
    with open(os.path.join(init_dir, component_init_src), "w") as f:
        f.write(init_codes)

def main():
    with open(sys.argv[1], "r") as f:
        config_mk_str = f.read()

        process_component_init(config_mk_str, sys.argv[2])
        process_config_mk(config_mk_str, sys.argv[1])

if __name__ == "__main__":
    main()



