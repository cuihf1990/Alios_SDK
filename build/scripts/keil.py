import os
import sys
import string

import xml.etree.ElementTree as etree
from xml.etree.ElementTree import SubElement
from utils import xml_indent
from config_mk import Projects

fs_encoding = sys.getfilesystemencoding()

def _get_filetype(fn):
    if fn.rfind('.cpp') != -1 or fn.rfind('.cxx') != -1:
        return 8

    if fn.rfind('.c') != -1 or fn.rfind('.C') != -1:
        return 1

    # assemble file type
    if fn.rfind('.s') != -1 or fn.rfind('.S') != -1:
        return 2

    # header type
    if fn.rfind('.h') != -1:
        return 5

    if fn.rfind('.lib') != -1:
        return 4

    # other filetype
    return 5

    
#ProjectFiles use for remove same name files
#add files
def MDKAddGroup(parent, name, files, project_path):
    group = SubElement(parent, 'Group')
    group_name = SubElement(group, 'GroupName')
    group_name.text = name

    for f in files:
        files = SubElement(group, 'Files')
        file = SubElement(files, 'File')
        file_name = SubElement(file, 'FileName')
        name = os.path.basename(f)

        file_name.text = name.decode(fs_encoding)
        file_type = SubElement(file, 'FileType')
        file_type.text = '%d' % _get_filetype(name)
        file_path = SubElement(file, 'FilePath')
        file_path.text = (aos_relative_path + f).decode(fs_encoding)

    return group

def MDKProject(tree, target, script):
    project_path = os.path.dirname(os.path.abspath(target))

    root = tree.getroot()
    out = file(target, 'wb')
    out.write('<?xml version="1.0" encoding="UTF-8" standalone="no" ?>\n')
    
    #change target name
    TargetName = tree.find('Targets/Target/TargetName')
    TargetName.text = buildstring
    OutputName = tree.find('Targets/Target/TargetOption/TargetCommonOption/OutputName')
    OutputName.text = buildstring
    
    # add group
    groups = tree.find('Targets/Target/Groups')
    if groups is None:
        groups = SubElement(tree.find('Targets/Target'), 'Groups')
    groups.clear() # clean old groups
    for group in script:
        # don't add an empty group
        if len(group['src']) != 0:
            group_tree = MDKAddGroup(groups, group['name'], group['src'], project_path)

            # add GroupOption
            GroupOption     = SubElement(group_tree,  'GroupOption')
            GroupArmAds     = SubElement(GroupOption, 'GroupArmAds')
            Cads            = SubElement(GroupArmAds, 'Cads')
            VariousControls = SubElement(Cads, 'VariousControls')
            MiscControls    = SubElement(VariousControls, 'MiscControls')
            MiscControls.text = '--via '+opt_dir+group['name']+'.c_opts'
            
            Aads            = SubElement(GroupArmAds, 'Aads')
            VariousControls = SubElement(Aads, 'VariousControls')
            MiscControls    = SubElement(VariousControls, 'MiscControls')
            MiscControls.text = '--via '+opt_dir+group['name']+'.as_opts'
    
    # set <OutputName>B-L475E-IOT01</OutputName> 
    
    xml_indent(root)
    out.write(etree.tostring(root, encoding='utf-8'))
    out.close()

def MDK5Project(target, script):
    template_tree = etree.parse('build/scripts/template.uvprojx')
    MDKProject(template_tree, target, script)
    opt_file = target.replace('.uvprojx', '.uvoptx')
    opt_tree = etree.parse('build/scripts/template.uvoptx')
    TargetName = opt_tree.find('Target/TargetName')
    TargetName.text = buildstring
    out = file(opt_file, 'wb')
    out.write(etree.tostring(opt_tree.getroot(), encoding='utf-8'))
    out.close()

'''
Projects = [
{'name':'alicrypto', 
'src':[ 
'a.c',
'a_1.s',
]
},
{'name':'alinkapp', 
'src':[ 
'./example/alinkapp/alink_sample.c',
]
}
]
'''

#argv[1]: buildstring, eg: nano@b_l475e
buildstring = sys.argv[1]
proj_output_dir = 'projects/autogen/'+buildstring+'/keil_project'
#use in xml text
aos_relative_path = '../../../../'
projectPath = proj_output_dir+'/'+buildstring+'.uvprojx'
opt_dir = 'opts/'

print 'Making keil project '+buildstring
MDK5Project(projectPath, Projects)
print 'keil project: '+ projectPath + ' has generated over'
