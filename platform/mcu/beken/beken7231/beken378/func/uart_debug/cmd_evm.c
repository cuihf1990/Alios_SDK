#include "include.h"
#include "cmd_evm.h"
#include "mem_pub.h"
#include "str_pub.h"
#include "uart_debug_pub.h"
#include "tx_evm_pub.h"

#include "udebug.h"
#include "uart_pub.h"
#include "schedule_pub.h"

#if CFG_SUPPORT_CALIBRATION
#include "bk7011_cal_pub.h"
#endif

#if CFG_TX_EVM_TEST
static UINT32 evm_translate_tx_rate(UINT32 rate)
{
    UINT32 param;

    switch(rate)
    {
    case 1 :
        param = 0x0;
        break;  // 1Mbps
    case 2 :
        param = 0x1;
        break;  // 2Mbps
    case 5 :
        param = 0x2;
        break;	// 5.5Mbps
    case 11:
        param = 0x3;
        break;	// 11Mbps
    case 6 :
        param = 0x4;
        break;	// 6Mbps
    case 9 :
        param = 0x5;
        break;	// 9Mbps
    case 12:
        param = 0x6;
        break;	// 12Mbps
    case 18:
        param = 0x7;
        break;	// 18Mbps
    case 24:
        param = 0x8;
        break;	// 24Mbps
    case 36:
        param = 0x9;
        break;	// 36Mbps
    case 48:
        param = 0xa;
        break;	// 48Mbps
    case 54:
        param = 0xb;
        break;	// 54Mbps
    default:
        param = 0xb;
        break;	// 54Mbps
    }

    return param;
}
#endif

/*txevm [-m mode] [-c channel] [-l packet-length] [-r physical-rate]*/
int do_evm(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    char cmd0 = 0;
    char cmd1 = 0;
    UINT8 fail = 0;
    UINT32 packet_len = EVM_DEFUALT_PACKET_LEN;
    UINT32 channel = EVM_DEFUALT_CHANNEL;
    UINT32 mode = EVM_DEFUALT_MODE;
    UINT32 rate = EVM_DEFUALT_RATE;
    UINT32 arg_id = 1;
    UINT32 arg_cnt = argc;

    /*step0, parameter conversion*/
    while(arg_cnt > 1)
    {
        if(arg_cnt > 1)
        {
            cmd0 = argv[arg_id][0];
            cmd1 = argv[arg_id][1];
        }

        switch(cmd0)
        {
        case '-':
        {
            arg_cnt -= 1;

            if(arg_cnt < 1)
            {
                fail = 1;
                break;
            }

            arg_cnt -= 1;
            switch(cmd1)
            {
            case 'm':
                mode = os_strtoul(argv[arg_id + 1], NULL, 10);
                break;

            case 'c':
                channel = os_strtoul(argv[arg_id + 1], NULL, 10);
                break;

            case 'l':
                packet_len = os_strtoul(argv[arg_id + 1], NULL, 10);
                break;

            case 'r':
                rate = os_strtoul(argv[arg_id + 1], NULL, 10);
                break;

            default:
                fail = 1;
                break;
            }
        }
        break;

        default:
            fail = 1;
            break;
        }

        if(fail)
        {
            return 1;
        }

        arg_id += 2;
    }

    /*step1, parameter check*/
    if(!(((1 == mode)
            || (0 == mode))
            && ((2412 <= channel)
                && (2484 >= channel))
            && ((0 < packet_len)
                && (4095 >= packet_len))
            && ((1 == rate)
                || (2 == rate)
                || (5 == rate)
                || (6 == rate)
                || (9 == rate)
                || (11 == rate)
                || (12 == rate)
                || (18 == rate)
                || (24 == rate)
                || (36 == rate)
                || (48 == rate)
                || (54 == rate))))
    {
        return 1;
    }

    /*step2, handle*/
#if CFG_TX_EVM_TEST
    if(mode)
    {
        evm_bypass_mac_set_tx_data_length(1, packet_len);
        evm_bypass_mac_set_rate(rate);
        evm_bypass_mac_set_channel(channel);

        evm_bypass_mac_test();

#if CFG_SUPPORT_CALIBRATION
        rwnx_cal_set_txpwr_by_rate(evm_translate_tx_rate(rate));
#endif

        UD_PRT("control c can exit the evm test\r\n");
    }
    else
    {
        evm_via_mac_set_rate((HW_RATE_E)rate, 1);
        evm_via_mac_set_channel(channel);

        evm_via_mac_begin();
    }
#endif // CFG_TX_EVM_TEST

    return 0;
}


// eof

