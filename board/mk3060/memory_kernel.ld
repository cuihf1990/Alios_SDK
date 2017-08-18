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
 * |data        |  kernel:392k
 * |            |
 * +------------+  0x0075200
 * |text        |
 * |data        |  app:264k
 * |            |
 * +------------+  0x00B7200
 * |            |
 * |            |
 * |  OTA TEMP  |
 * |            |  291k
 * |            |
 * |            |  
 * +------------+  0x100000
*/


/* Split memory into area for vectors and ram */
MEMORY
{
	flash  (rx) : ORIGIN = 0x00012000, LENGTH = 368k
	ram   (rwx): ORIGIN = 0x00410020, LENGTH = 256k - 64k - 32
}
