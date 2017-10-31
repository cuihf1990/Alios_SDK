import os, sys, time
from autotest import Autotest

def main(filename='~/lb-all.bin'):
    ap_ssid = 'aos_test_01'
    ap_pass = 'Alios@Embedded'
    server = '10.125.52.132'
    port = 34568

    #parse input
    i = 1
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg.startswith('--firmware='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --firmware=firmware.bin'.format(arg)
            filename = args[1]
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
    logname = 'line_topology-' + logname +'.log'
    if at.start(server, port, logname) == False:
        print 'error: start failed'
        return [1, 'connect testbed failed']

    #request device allocation
    type = 'mxchip'
    number = 5
    timeout = 3600
    allocted = at.device_allocate(type, number, timeout)
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
        succeed = False; retry = 5
        print 'programming device {0} ...'.format(devices[device])
        for i in range(retry):
            if at.device_program(device, '0x13200', filename) == True:
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
            mac =  at.device_run_cmd(device, ['reboot'], 1, 1.5, ['mac'])
            if mac != False and mac != []:
                mac = mac[0].replace('mac ', '')
                mac = mac.replace(':', '')
                mac = mac.replace(' ', '0')
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

    #setup line topology
    print "topology:"
    print "A <--> B <--> C <--> D <--> E\n"
    for i in range(len(device_list)):
        device = device_list[i]
        at.device_run_cmd(device, ['umesh', 'whitelist', 'clear'])
        if (i-1) >= 0:
            prev_dev = device_list[i-1]
            at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr[prev_dev]['mac']+'0000'])
        if (i+1) < len(device_list):
            next_dev = device_list[i+1]
            at.device_run_cmd(device, ['umesh', 'whitelist', 'add', device_attr[next_dev]['mac']+'0000'])
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
                return [1, 'connect alink failed']
        else:
            at.device_run_cmd(device, ['umesh', 'start'])
            time.sleep(5)

        if i == 0:
            expected_state = 'leader'
        elif i < len(device_list) - 1:
            expected_state = 'router'
        else:
            expected_state = 'leaf'

        succeed = False; retry = 5
        while retry > 0:
            state = at.device_run_cmd(device, ['umesh', 'state'], 1, 1, filter)
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
            return [1, 'connect mesh failed']

        succeed = False; retry = 3
        while retry > 0:
            ipaddr = at.device_run_cmd(device, ['umesh', 'ipaddr'], 2, 1.5, ['.'])
            if ipaddr == False or ipaddr == [] or len(ipaddr) != 2:
                retry -= 1
                continue
            ipaddr[0] = ipaddr[0].replace('\t', '')
            ipaddr[1] = ipaddr[1].replace('\t', '')
            device_attr[device]['ipaddr'] = ipaddr[0:2]
            succeed = True
            break;
        if succeed == False:
            print 'error: get ipaddr for device {0} failed'.format(device)
            return [1, 'get ipaddr failed']

    for device in device_list:
        print "{0}:{1}".format(device, device_attr[device])

    retry = 20
    #ping
    print 'test connectivity with icmp:'
    success_num = 0; fail_num = 0
    for device in device_list:
        for other in device_list:
            if device == other:
                continue
            for pkt_len in ['20', '500', '1000']:
                filter = ['bytes from']
                dst_ip = device_attr[other]['ipaddr'][0]
                for i in range(retry):
                    response = at.device_run_cmd(device, ['umesh', 'ping', dst_ip, pkt_len], 1, 1.5, filter)
                    expected_response = '{0} bytes from {1}'.format(pkt_len, dst_ip)
                    if response == False or response == [] or expected_response not in response[0]:
                        if i < retry - 1:
                            continue
                        else:
                            print '{0} ping {1} with {2} bytes by local ip addr failed'.format(device, other, pkt_len)
                            fail_num += 1
                            break
                    else:
                        success_num += 1
                        break

    print 'ping: succeed-{0}, failed-{1}'.format(success_num, fail_num)

    #udp
    print '\ntest connectivity with udp:'
    success_num = 0; fail_num = 0
    for device in device_list:
        for other in device_list:
            if device == other:
                continue
            for pkt_len in ['20', '500', '1000']:
                dst_ip = device_attr[other]['ipaddr'][0]
                filter = ['bytes autotest echo reply from']
                for i in range(retry):
                    response = at.device_run_cmd(device, ['umesh', 'autotest', dst_ip, '1', pkt_len], 1, 1, filter)
                    expected_response = '{0} bytes autotest echo reply from {1}'.format(pkt_len, dst_ip)
                    if response == False or response == [] or expected_response not in response[0]:
                        if i < retry - 1:
                            continue
                        else:
                            print '{0} send {1} with {2} bytes by local ip addr failed'.format(device, other, pkt_len)
                            fail_num += 1
                            break
                    else:
                        success_num += 1
                        break

    print 'udp: succeed-{0}, failed-{1}'.format(success_num, fail_num)

    at.stop()
    return [0, 'success']

if __name__ == '__main__':
    [code, msg] = main()
    sys.exit(code)

