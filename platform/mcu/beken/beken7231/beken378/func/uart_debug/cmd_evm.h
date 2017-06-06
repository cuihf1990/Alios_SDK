#ifndef _CMD_EVM_H_
#define _CMD_EVM_H_

#include "command_table.h"

extern int do_evm(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]);

#define CMD_EVM_MAXARG                          10

#define EVM_DEFUALT_MODE                       (1)
#define EVM_DEFUALT_PACKET_LEN                 (512)
#define EVM_DEFUALT_RATE                       (54)
#define EVM_DEFUALT_CHANNEL                    (2437)


#define ENTRY_CMD_EVM               \
	ENTRY_CMD(txevm,                          \
				CMD_EVM_MAXARG,     \
				1,                             \
				do_evm,             \
				"txevm [-m mode] [-c channel] [-l packet-length] [-r physical-rate]\r\n",         \
				"Options:\r\n"\
				"     -m mode: 1,0                       1: tx packet bypass mac, 0: via mac\r\n"\
				"     -c channel: 1,2,...,14             channel number\r\n"\
				"     -l packet-length: 0--4095          legacy:0--4095 ht:0--65535 vht:0--1048575\r\n"\
				"     -r ppdu-rate: 1,2,5,6,9,11,12,18,24,36,48,54    Mbps\r\n"\
				)

#endif // _CMD_EVM_H_
