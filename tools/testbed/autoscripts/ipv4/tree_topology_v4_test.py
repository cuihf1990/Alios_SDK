import sys, os, time
from autotest import Autotest

def restore_extnetid(at, device_list):
    #restore extnetid to default
    extnetid = '010203040506'
    for device in device_list:
        at.device_run_cmd(device, ['umesh', 'extnetid', extnetid])
        at.device_run_cmd(device, ['umesh', 'whitelist', 'disable'])

def main(firmware='~/lb-all.bin', model='mk3060'):
    ap_ssid = 'aos_test_01'
    ap_pass = 'Alios@Embedded'
    server = '10.125.52.132'
    port = 34568
    models={'mk3060':'0x13200', 'esp32':'0x10000'}

    #parse input
    i = 1
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg.startswith('--firmware='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --firmware=firmware.bin'.format(arg)
            firmware = args[1]
        elif arg.startswith('--model='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --model=mk3060'.format(arg)
            model = args[1]
        elif arg.startswith('--wifissid='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --wifissid=test_wifi'.format(arg)
            ap_ssid = args[1]
        elif arg.startswith('--wifipass='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --wifipass=test_password'.format(arg)
            ap_pass = args[1]
        elif arg=='--help':
            print 'Usage: python {0} [--firmware=xxx.bin] [--wifissid=wifi_ssid] [--wifipass=password]'.format(sys.argv[0])
            return [0, 'help']
        i += 1

    at=Autotest()
    logname=time.strftime('%Y-%m-%d@%H-%M')
    logname = 'tree_topology-' + logname +'.log'
    if at.start(server, port, logname) == False:
        print 'error: start failed'
        return [1, 'connect testbed failed']

    #request device allocation
    if not model or model.lower() not in models:
        print "error: unsupported model {0}".format(repr(model))
        return [1, "model {0} error".format(repr(model))]
    model = model.lower()
    number = 7
    timeout = 3600
    allocted = at.device_allocate(model, number, timeout)
    if len(allocted) != number:
        print "error: request device allocation failed"
        return [1, 'allocate device failed']
    devices = {}
    for i in range(len(allocted)):
        devices[chr(ord('A')+i)] = allocted[i]
    device_list = list(devices)
    device_list.sort()
    device_attr={}

    if at.device_subscribe(devices) == False:
        print 'error: subscribe to device failed, some devices may not exist in testbed'
        return [1, 'subscribe devices failed']
    for device in device_list:
        at.device_control(device, 'reset')
    time.sleep(3)

    #program devices
    for device in device_list:
        addr = models[model]
        succeed = False; retry = 5
        print 'programming device {0} ...'.format(devices[device])
        for i in range(retry):
            if at.device_program(device, addr, firmware) == True:
                succeed = True
                break
            time.sleep(0.5)
        if succeed == False:
            print 'error: program device {0} failed'.format(devices[device])
            return [1, 'program device failed']
        print 'program device {0} succeed'.format(devices[device])

    #reboot and get device mac address
    retry = 5
    for device in device_list:
        succeed = False
        for i in range(retry):
            at.device_control(device, 'reset')
            time.sleep(2.5)
            at.device_run_cmd(device, ['netmgr', 'clear'])
            at.device_run_cmd(device, ['kv', 'del', 'alink'])
            mac =  at.device_run_cmd(device, ['mac'], 1, 1.5, ['MAC address:'])
            at.device_control(device, 'reset')
            if mac and len(mac) == 1:
                mac = mac[0].split()[-1]
                mac = mac.replace('-', '')
                device_attr[device] = {'mac':mac}
                succeed = True
                break;
        if succeed == False:
            print 'error: reboot and get mac addr for device {0} failed'.format(device)
            return [1, 'get macaddr failed']
    time.sleep(5)

    bytes = os.urandom(6)
    extnetid = ''
    for byte in bytes:
        extnetid = extnetid + '{0:02x}'.format(ord(byte))
    for device in device_list:
        at.device_run_cmd(device, ['umesh', 'extnetid', extnetid])
        at.device_run_cmd(device, ['umesh', 'stop'])

    #setup tree topology
    print 'topology:'
    print '       A'
    print '    /     \\'
    print '   B       C'
    print ' /   \   /   \\'
    print 'D     E F     G\n'
    device = 'A'
    at.device_run_cmd(device, ['umesh', 'whitelist', 'clear'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr['B']['mac']+'0000'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr['C']['mac']+'0000'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'enable'])
    at.device_run_cmd(device, ['umesh', 'whitelist'])
    device = 'B'
    at.device_run_cmd(device, ['umesh', 'whitelist', 'clear'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr['A']['mac']+'0000'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr['D']['mac']+'0000'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr['E']['mac']+'0000'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'enable'])
    at.device_run_cmd(device, ['umesh', 'whitelist'])
    device = 'C'
    at.device_run_cmd(device, ['umesh', 'whitelist', 'clear'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr['A']['mac']+'0000'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr['F']['mac']+'0000'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr['G']['mac']+'0000'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'enable'])
    at.device_run_cmd(device, ['umesh', 'whitelist'])
    device = 'D'
    at.device_run_cmd(device, ['umesh', 'whitelist', 'clear'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr['B']['mac']+'0000'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'enable'])
    at.device_run_cmd(device, ['umesh', 'whitelist'])
    device = 'E'
    at.device_run_cmd(device, ['umesh', 'whitelist', 'clear'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr['B']['mac']+'0000'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'enable'])
    at.device_run_cmd(device, ['umesh', 'whitelist'])
    device = 'F'
    at.device_run_cmd(device, ['umesh', 'whitelist', 'clear'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr['C']['mac']+'0000'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'enable'])
    at.device_run_cmd(device, ['umesh', 'whitelist'])
    device = 'G'
    at.device_run_cmd(device, ['umesh', 'whitelist', 'clear'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr['C']['mac']+'0000'])
    at.device_run_cmd(device, ['umesh', 'whitelist', 'enable'])
    at.device_run_cmd(device, ['umesh', 'whitelist'])

    #start devices
    filter = ['disabled', 'detached', 'attached', 'leaf', 'router', 'super_router', 'leader', 'unknown']
    for i in range(len(device_list)):
        device = device_list[i]
        if i == 0:
            at.device_run_cmd(device, ['netmgr', 'connect', ap_ssid, ap_pass])
            time.sleep(12)
            uuid = at.device_run_cmd(device, ['uuid'], 1, 1.5, ['uuid:', 'not connected'])
            if uuid == False or len(uuid) != 1 or 'uuid:' not in uuid[0]:
                print 'error: connect device to alink failed, response = {0}'.format(uuid)
                restore_extnetid(at, device_list)
                return [1, 'connect alink failed']
        else:
            at.device_run_cmd(device, ['umesh', 'start'])
            time.sleep(5)

        if i == 0:
            expected_state = 'leader'
        else:
            expected_state = 'router'

        succeed = False; retry = 5
        while retry > 0:
            state = at.device_run_cmd(device, ['umesh', 'state'], 1, 1.5, filter)
            if state == [expected_state]:
                succeed = True
                break
            at.device_run_cmd(device, ['umesh', 'stop'], 1, 1)
            at.device_run_cmd(device, ['umesh', 'start'], 5, 1)
            time.sleep(5)
            retry -= 1
        if succeed == True:
            print '{0} connect to mesh as {1} succeed'.format(device, expected_state)
        else:
            print 'error: {0} connect to mesh as {1} failed'.format(device, expected_state)
            restore_extnetid(at, device_list)
            return [1, 'connect mesh failed']

        succeed = False; retry = 3
        while retry > 0:
            ipaddr = at.device_run_cmd(device, ['umesh', 'ipaddr'], 2, 1, ['.'])
            if ipaddr == False or ipaddr == [] or len(ipaddr) != 2:
                continue
            ipaddr[0] = ipaddr[0].replace('\t', '')
            ipaddr[1] = ipaddr[1].replace('\t', '')
            device_attr[device]['ipaddr'] = ipaddr[0:2]
            succeed = True
            break;
        if succeed == False:
            print 'error: get ipaddr for device {0} failed'.format(device)
            restore_extnetid(at, device_list)
            return [1, 'get ipaddr failed']
    for device in device_list:
        print "{0}:{1}".format(device, device_attr[device])

    retry = 5
    #ping
    print 'test connectivity with icmp:'
    ping_pass_num = 0; ping_fail_num = 0
    for device in device_list:
        for other in device_list:
            if device == other:
                continue
            for pkt_len in ['20', '500', '1000']:
                filter = ['bytes from']
                dst_ip = device_attr[other]['ipaddr'][0]
                for i in range(retry):
                    response = at.device_run_cmd(device, ['umesh', 'ping', dst_ip, pkt_len], 1, 0.5, filter)
                    expected_response = '{0} bytes from {1}'.format(pkt_len, dst_ip)
                    if response == False or response == [] or expected_response not in response[0]:
                        if i < retry - 1:
                            continue
                        else:
                            print '{0} ping {1} with {2} bytes by local ip addr failed'.format(device, other, pkt_len)
                            ping_fail_num += 1
                            break
                    else:
                        ping_pass_num += 1
                        break
    print 'ping: pass-{0}, fail-{1}'.format(ping_pass_num, ping_fail_num)

    #udp
    print '\ntest connectivity with udp:'
    upd_pass_num = 0; udp_fail_num = 0
    for device in device_list:
        for other in device_list:
            if device == other:
                continue
            for pkt_len in ['20', '500', '1000']:
                dst_ip = device_attr[other]['ipaddr'][0]
                filter = ['bytes autotest echo reply from']
                for i in range(retry):
                    response = at.device_run_cmd(device, ['umesh', 'autotest', dst_ip, '1', pkt_len], 1, 0.5, filter)
                    expected_response = '{0} bytes autotest echo reply from {1}'.format(pkt_len, dst_ip)
                    if response == False or response == [] or expected_response not in response[0]:
                        if i < retry - 1:
                            continue
                        else:
                            print '{0} send {1} with {2} bytes by local ip addr failed'.format(device, other, pkt_len)
                            udp_fail_num += 1
                            break
                    else:
                        upd_pass_num += 1
                        break
    print 'udp: pass-{0}, fail-{1}'.format(upd_pass_num, udp_fail_num)

    restore_extnetid(at, device_list)
    at.stop()
    return [0, 'success! ping: pass-{0} fail-{1}, udp: pass-{2} fail-{3}'.format(ping_pass_num, ping_fail_num, upd_pass_num, udp_fail_num)]

if __name__ == '__main__':
    [code, msg] = main()
    sys.exit(code)

