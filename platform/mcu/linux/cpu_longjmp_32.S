##
 # Copyright (C) 2016 YunOS Project. All rights reserved.
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #   http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
##

#
# rhino_setjmp/rhino_longjmp for the i386 architecture
#

#
# The jmp_buf is assumed to contain the following, in order:
#    %ebx
#    %esp
#    %ebp
#    %esi
#    %edi
#    <return address>
#

    .text
    .align 4
    .globl rhino_setjmp
    .type rhino_setjmp, @function
rhino_setjmp:
    movl 4(%esp),%edx
    popl %ecx          # Return address, and adjust the stack
    xorl %eax,%eax     # Return value
    movl %ebx,(%edx)
    movl %esp,4(%edx)  # Post-return %esp!
    pushl %ecx         # Make the call/return stack happy
    movl %ebp,8(%edx)
    movl %esi,12(%edx)
    movl %edi,16(%edx)
    movl %ecx,20(%edx) # Return address
    ret

    .size rhino_setjmp,.-rhino_setjmp

    .text
    .align 4
    .globl rhino_longjmp
    .type rhino_longjmp, @function
rhino_longjmp:
    movl 4(%esp),%edx  # jmp_ptr address
    movl 8(%esp),%eax  # Return value
    movl (%edx),%ebx
    movl 4(%edx),%esp
    movl 8(%edx),%ebp
    movl 12(%edx),%esi
    movl 16(%edx),%edi
    jmp *20(%edx)

    .size rhino_longjmp,.-rhino_longjmp
