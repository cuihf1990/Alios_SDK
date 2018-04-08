#ifndef ACTIVATION_H
#define ACTIVATION_H

#ifndef UINT32
#define UINT32 unsigned int
#endif

#ifndef UINT8
#define UINT8 unsigned char
#endif

/*
	input: version 4byte + random 4 byte + mac 4byte + chip_code 4byte
	output: output_buffer store the version info process, 65 byte
	return: 0 success, 1 failed	
*/
extern UINT32 aos_get_version_info (UINT8 version_num[4], UINT8 random_num[4], UINT8 mac_address[4], UINT8 chip_code[4], UINT8 *output_buffer, UINT32 output_buffer_size);

#endif

