/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
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

#ifndef YOS_SYSCALL_UAPI_H
#define YOS_SYSCALL_UAPI_H

extern const void *g_syscall_tbl[];

#define SYSCALL_TBL g_syscall_tbl

#define SYS_CALL0(nr, t) ((t (*)(void))(SYSCALL_TBL[nr]))()

#define SYS_CALL1(nr, t, t1, p1) ((t (*)(t1))(SYSCALL_TBL[nr]))(p1)

#define SYS_CALL2(nr, t, t1, p1, t2, p2) ((t (*)(t1, t2))(SYSCALL_TBL[nr]))(p1, p2)

#define SYS_CALL3(nr, t, t1, p1, t2, p2, t3, p3) ((t (*)(t1, t2, t3))(SYSCALL_TBL[nr]))(p1, p2, p3)

#define SYS_CALL4(nr, t, t1, p1, t2, p2, t3, p3, t4, p4) ((t (*)(t1, t2, t3, t4))(SYSCALL_TBL[nr]))(p1, p2, p3, p4)

#define SYS_CALL5(nr, t, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5) ((t (*)(t1, t2, t3, t4, t5))(SYSCALL_TBL[nr]))(p1, p2, p3, p4, p5)

#define SYS_CALL6(nr, t, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6) ((t (*)(t1, t2, t3, t4, t5, t6))(SYSCALL_TBL[nr]))(p1, p2, p3, p4, p5, p6)

#endif

