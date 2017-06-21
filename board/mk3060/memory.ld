/*
 * +------------+  0x0040000
 * Vect redirect   32
 * +------------+  
 *
 * +------------+  0x00400020
 * data         |
 * end
 * |(heap)      |
 * .            .
 * .            .
 * |(heap limit)|
 *
 * |- - - - - - |
 * stack bottom    256k
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
 * |data        |  984k
 * |            |  
 * +------------+  0x108800
 * |            |
 * |            |
 * |  OTA TEMP  |
 * |            |  984k
 * |            |
 * |            |  
 * +------------+  0x1FF000
 * |   Para 3   |   4k
 * +------------+ 
 */


/* Split memory into area for vectors and ram */
MEMORY
{
	flash  (rx) : ORIGIN = 0x0012000, LENGTH = 984k
	ram   (rwx): ORIGIN = 0x00400020, LENGTH = 256k - 32
}