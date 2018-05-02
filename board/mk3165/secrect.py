# void Board_SecrectUpdate(int argc, char **argv)
# {
#     uint32_t offset = 0x00;
#     char magic[2] = {'I','D'};
#     hal_flash_write(HAL_PARTITION_LINK_KEY, &offset, magic, 2);

#     uint8_t len = strlen(argv[1])+1+strlen(argv[2])+1+strlen(argv[3])+1+strlen(argv[4])+1;
#     hal_flash_write(HAL_PARTITION_LINK_KEY, &offset, &len, 1);

#     hal_flash_write(HAL_PARTITION_LINK_KEY, &offset, argv[1], strlen(argv[1])+1);
#     hal_flash_write(HAL_PARTITION_LINK_KEY, &offset, argv[2], strlen(argv[2])+1);
#     hal_flash_write(HAL_PARTITION_LINK_KEY, &offset, argv[3], strlen(argv[3])+1);
#     hal_flash_write(HAL_PARTITION_LINK_KEY, &offset, argv[4], strlen(argv[4])+1);

#     uint16_t crc;
#     CRC16_Context contex;
#     CRC16_Init( &contex );
#     CRC16_Update( &contex, argv[1], strlen(argv[1])+1);
#     CRC16_Update( &contex, argv[2], strlen(argv[2])+1);
#     CRC16_Update( &contex, argv[3], strlen(argv[3])+1);
#     CRC16_Update( &contex, argv[4], strlen(argv[4])+1);
#     CRC16_Final( &contex, &crc );
#     hal_flash_write(HAL_PARTITION_LINK_KEY, &offset, &crc, 2);
# }

import sys
import struct

def crc16(data):
    wcrc = 0
    for i in data:
        c = i
        for j in range(8):
            treat = c & 0x80
            c <<= 1
            bcrc = (wcrc >> 8) & 0x80
            wcrc <<= 1
            wcrc = wcrc & 0xffff
            if (treat != bcrc):
                wcrc ^= 0x1021
    return wcrc

pk, ps, ds, dn = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]

data = pk.encode()+b'\x00' + ps.encode()+b'\x00' + ds.encode()+b'\x00' + dn.encode()+b'\x00'
crc = crc16(data)
img = b'ID' + struct.pack('<B', len(data)) + data + struct.pack('<H', crc)
with open('secrect.bin', 'wb') as f:
    f.write(img)