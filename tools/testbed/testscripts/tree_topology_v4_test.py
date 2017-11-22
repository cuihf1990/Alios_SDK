import sys, os, time
from autotest import Autotest
from mesh_common import *

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
    number = 7
    timeout = 3600
    allocated = allocate_devices(at, model, number, timeout)
    if len(allocated) != number:
        return [1, 'allocate device failed']

    devices = {}
    for i in range(len(allocated)):
        devices[chr(ord('A')+i)] = allocated[i]
    device_list = list(devices)
    device_list.sort()
    device_attr={}

    #subscribe and reboot devices
    result = subscribe_and_reboot_devices(at, devices)
    if result == False:
        return [1, 'subscribe devices failed']

    #program devices
    result = program_devices(at, devices, model, firmware)
    if result == False:
        return [1, 'program device failed']

    #reboot and get device mac address
    result = reboot_and_get_mac(at, device_list, device_attr)
    if result == False:
        return [1, 'reboot and get macaddr failed']

    #set random extnetid and stop mesh
    set_random_extnetid_and_stop_mesh(at, device_list)

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

    #setup whitelist for tree topology
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

    #start devices to form mesh network
    filter = ['disabled', 'detached', 'attached', 'leaf', 'router', 'super_router', 'leader', 'unknown']
    for i in range(len(device_list)):
        device = device_list[i]
        if i == 0:
            at.device_run_cmd(device, ['netmgr', 'connect', ap_ssid, ap_pass])
            time.sleep(12)
            uuid = at.device_run_cmd(device, ['uuid'], 1, 1.5, ['uuid:', 'not connected'])
            if uuid == False or len(uuid) != 1 or 'uuid:' not in uuid[0]:
                print 'error: connect device {0} to alink failed, response = {1}'.format(devices[device], uuid)
                restore_device_status(at, device_list)
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
            restore_device_status(at, device_list)
            return [1, 'form desired mesh network failed']

    #get device ips
    get_device_ips(at, device_list, device_attr)

    #print device attributes
    print_device_attrs(device_attr)

    #ping test
    [ping_pass_num, ping_fail_num] = ping_test(at, device_list, device_attr)

    #udp test
    [udp_pass_num, udp_fail_num] = udp_test(at, device_list, device_attr)
    retry = 5

    restore_device_status(at, device_list)
    at.stop()
    return [0, 'succeed. ping: pass-{0} fail-{1}, udp: pass-{2} fail-{3}'.format(ping_pass_num, ping_fail_num, udp_pass_num, udp_fail_num)]

if __name__ == '__main__':
    [code, msg] = main()
    sys.exit(code)

