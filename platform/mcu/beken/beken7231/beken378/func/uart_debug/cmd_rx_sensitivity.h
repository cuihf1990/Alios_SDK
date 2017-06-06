#ifndef _CMD_RX_SENSITIVITY_H_
#define _CMD_RX_SENSITIVITY_H_

#include "command_table.h"

extern int do_rx_sensitivity(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]);

#define CMD_RX_SENSITIVITY_MAXARG              (8)
#define RXSENS_DEFUALT_MODE                    (0)
#define RXSENS_DEFUALT_DURATION                (20)
#define RXSENS_DEFUALT_CHANNEL                 (6)


#define ENTRY_CMD_RX_SENSITIVITY               \
	ENTRY_CMD(rxsens,                          \
				CMD_RX_SENSITIVITY_MAXARG,     \
				1,                             \
				do_rx_sensitivity,             \
				"rxsens [-m mode] [-d duration] [-c channel] [-l lost]\r\n"         \
				"      test rx sensitivity, and get statistic result about fcs, per and so on\r\n", \
				"Options:\r\n" \
				"      -m mode                  0:PHY_CHNL_BW_20  1:PHY_CHNL_BW_40 \r\n"\
				"      -d duration              duration: n second\r\n"\
				"      -c channel: 1,2,...,14   channel number\r\n"\
				"      -l don't show rxsens log any more\r\n"\
				"\r\n")

#endif // _CMD_RX_SENSITIVITY_H_
