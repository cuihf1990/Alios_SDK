To execute the ysh, please :
1  move the tools/Makefile_RSH to the root directory of rhino 
2  change the rhino/vendor/host/linux/main.c function call 
   from test_case_task_start() to ysh_task_start() 
   and do not forget: extern void ysh_task_start(void); 
3  use the command:make -f Makefile_RSH
