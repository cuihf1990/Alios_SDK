#!/bin/bash

## http://astyle.sourceforge.net/astyle.html

DIRS=(devices
      framework
      include
      kernel/modules
      kernel/protocols/mesh
      kernel/rhino
      kernel/vcall/yos
      utility/hashtable
      utility/digest_algorithm
      utility/base64
      security/tfs
      )

for DIR in ${DIRS[*]}
do

    find ./$DIR -name "*.[ch]" | xargs astyle --formatted --style=otbs --min-conditional-indent=0 --indent=spaces=4 --indent-switches --indent-col1-comments --pad-oper --pad-header --indent-col1-comments --max-instatement-indent=80 --max-code-length=80 --break-after-logical --align-pointer=name --add-brackets --convert-tabs --lineend=linux --suffix=none

done