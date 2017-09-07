import os, sys, time
sys.path.append('../../')
from autotest import Autotest

devices = {'A':'mxchip-DN02QRIQ', 'B':'mxchip-DN02QRIX', 'C':'mxchip-DN02QRJ6', 'D':'mxchip-DN02QRJ7', 'E':'mxchip-DN02QRJ9'}
device_list = list(devices)
device_attr={}
device_list.sort()
at=Autotest()
logname=time.strftime('%Y-%m-%d@%H-%M')
logname = 'line_topology-' + logname +'.log'
at.start('10.125.52.132', 34568, logname)
if at.device_subscribe(devices) == False:
    print 'error: subscribe to device failed, some devices may not exist in testbed'
    exit(1)

#reboot and get device mac address
for device in device_list:
    at.device_control(device, 'reset')
    time.sleep(2.5)
    at.device_run_cmd(device, ['netmgr', 'clear'])
    at.device_run_cmd(device, ['kv', 'delete', 'alink'])
    mac =  at.device_run_cmd(device, ['reboot'], 1, 1.5, ['mac'])
    if mac != False and mac != []:
        mac = mac[0].replace('mac ', '')
        print '{0} mac: {1}'.format(device, mac)
        mac = mac.replace(':', '')
        mac = mac.replace(' ', '0')
        device_attr[device] = {'mac':mac}
    else:
        print 'error: reboot and get mac addr for device {0} failed, ret={1}'.format(device, mac)
        exit(1)
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
for i in range(len(device_list)):
    device = device_list[i]
    if i == 0:
        at.device_run_cmd(device, ['netmgr', 'connect', 'aos_test_01', 'Alios@Embedded'])
        time.sleep(12)
        uuid = at.device_run_cmd(device, ['uuid'], 1, 1)
        if uuid == []:
            print 'error: alink connect to server failed'
            exit(1)
    else:
        at.device_run_cmd(device, ['umesh', 'start'])
        time.sleep(3)

    retry = 5
    filter = ['disabled', 'detached', 'attached', 'leaf', 'router', 'super_router', 'leader', 'unknown']
    while retry > 0:
        state = at.device_run_cmd(device, ['umesh', 'state'], 1, 1, filter)
        if i == 0:
            if state == ['leader']:
                print '{0} connect to mesh as leader succeed'.format(device)
                break
        elif i < len(device_list) - 1:
            if state == ['router']:
                print '{0} connect to mesh as router succeed'.format(device)
                break
        else:
            if state == ['leaf']:
                print '{0} connect to mesh as leaf succeed'.format(device)
                break
        at.device_run_cmd(device, ['umesh', 'stop'], 1, 1)
        at.device_run_cmd(device, ['umesh', 'start'], 5, 1)
        time.sleep(3)
        retry -= 1

    if retry == 0:
        print 'error: {0} connect to mesh failed'.format(device)
        exit(1)

    ipaddr = at.device_run_cmd(device, ['umesh', 'ipaddr'], 2, 1, ['.'])
    if ipaddr == False or ipaddr == [] or len(ipaddr) != 2:
        print 'error: get ipaddr for device {0} failed'.format(device)
        exit(1)
    else:
        ipaddr[0] = ipaddr[0].replace('\t', '')
        ipaddr[1] = ipaddr[1].replace('\t', '')
        device_attr[device]['ipaddr'] = ipaddr[0:3]
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

