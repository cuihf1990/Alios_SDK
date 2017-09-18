/*
 * +------------+  0x00400000
 * Vect redirect   32
 * +------------+  
 *
 * +------------+  0x00400020
 * data         |  64k
 * bss          |
 * +------------+  0x00410020
 * data         |
 * end
 * |(heap)      |
 * .            .  256k - 64k - 32
 * .            .
 * |(heap limit)|
 *
 * |- - - - - - |
 * stack bottom |
 * +------------+  
 *
 * +------------+  0x0000000
 * |Bootloader  |
 * |            |  64k 
 * +------------+  0x0010000
 * |   Para 1   |   4k
 * +------------+ 
 * |   Para 2   |   4k
 * +------------+  0x0012000
 * |   Para 3   |   4k
 * +------------+  0x0013000
 * +------------+  0x0013200
 * |vectors     |
 * |            |
 * |------------+
 * |text        |  
 * |data        |  kernel:363k
 * |            |
 * +------------+  0x006E000
 * +------------+  0x006E3C0
 * |text        |
 * |data        |  app:219k
 * |            |
 * +------------+  0x00A5000
 * +------------+  0x00A5380
 * |            |
 * |            |
 * |  OTA TEMP  |
 * |            |  363k
 * |            |
 * |            |  
 * +------------+  0x100000
*/

/* kernel's phy start address is 0x13200.
 * Flash size = 0x006E000-0x0013200 = 363K
 * Code size = 363K*32/34=341K
 */


/* Split memory into area for vectors and ram */
MEMORY
{
	flash  (rx) : ORIGIN = 0x00012000, LENGTH = 341k
	ram   (rwx): ORIGIN = 0x00410020, LENGTH = 256k - 64k - 32
}
