/**
 ******************************************************************************
 * @file    BootloaderEntrance.c
 * @author  William Xu
 * @version V2.0.0
 * @date    05-Oct-2014
 * @brief   MICO bootloader main entrance.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2014 MXCHIP Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */

#include <ctype.h>
#include "hal/soc/soc.h"
#include "board.h"
#include "bootloader.h"

extern void set_boot_ver(void);
extern int update(void);
#if STDIO_BREAK_TO_MENU
extern int stdio_break_in(void);
#endif
extern void boot(void);
extern void menu_loop(void);

const char menu[] =
"\r\n"
"MICO bootloader for %s, %s, HARDWARE_REVISION: %s\r\n"
"+ command -------------------------+ function ------------+\r\n"
"| 0:BOOTUPDATE    <-r>             | Update bootloader    |\r\n"
"| 1:FWUPDATE      <-r>             | Update application   |\r\n"
"| 2:DRIVERUPDATE  <-r>             | Update RF driver     |\r\n"
"| 3:PARUPDATE     <-id n><-r><-e>  | Update MICO partition|\r\n"
"| 4:FLASHUPDATE   <-dev device>    |                      |\r\n"
"|  <-e><-r><-start addr><-end addr>| Update flash content |\r\n"
"| 5:MEMORYMAP                      | List flash memory map|\r\n"
"| 6:BOOT                           | Excute application   |\r\n"
"| 7:REBOOT                         | Reboot               |\r\n"
"+----------------------------------+----------------------+\r\n"
"|    (C) COPYRIGHT 2015 MXCHIP Corporation  By William Xu |\r\n"
" Notes:\r\n"
" -e Erase only  -r Read from flash -dev flash device number\r\n"
"  -start flash start address -end flash start address\r\n"
" Example: Input \"4 -dev 0 -start 0x400 -end 0x800\": Update \r\n"
"          flash device 0 from 0x400 to 0x800\r\n";

int main(void)
{
  // set_boot_ver();

  // update();

#if STDIO_BREAK_TO_MENU
  if (stdio_break_in())
    goto MENU;
#endif

    // boot();

#if STDIO_BREAK_TO_MENU
  MENU:
#endif

  printf ( menu, MODEL, Bootloader_REVISION, HARDWARE_REVISION );

  for(;;)
  {                             
    menu_loop();
  }
}


