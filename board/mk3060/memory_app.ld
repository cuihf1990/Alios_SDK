/*
 * +------------+  0x00040000
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
 * 
 * +------------+  0x0000000
 * |Bootloader  |
 * |            |  64k 
 * +------------+  0x0010000
 * |   Para 1   |   4k
 * +------------+ 
 * |   Para 2   |   4k
 * +------------+  0x0012000
 * |vectors     |
 * |            |
 * |------------+
 * |text        |  
 * |data        |  kernel: 800k
 * |            |  
 * +------------+  0x00DA000
 * |text        |
 * |data        |  app: 384k
 * |            |
 * +------------+  0x0013A000
 * |  OTA TEMP  |
 * |            |   784k
 * |            |
 * |            |  
 * +------------+  0x001FE000
 * |   Para 3   |   4k
 * +------------+  0x001FF000
 * |   Para 4   |   4K
 * +------------+
 */



/* Split memory into area for vectors and ram */
MEMORY
{
	flash  (rx) : ORIGIN = 0x00DA000, LENGTH = 361k
	ram   (rwx): ORIGIN = 0x00400020, LENGTH = 64k
}
