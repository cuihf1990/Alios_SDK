/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ARCH_SETJMP_H
#define ARCH_SETJMP_H

struct jmp_buf_s {
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned int eip;
};

typedef struct jmp_buf_s jmp_buf[1];

#endif /* ARCH_SETJMP_H */

