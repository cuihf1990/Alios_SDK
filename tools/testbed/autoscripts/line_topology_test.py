import sys, time
sys.path.append('../')
from autotest import Autotest

devices = {'A':'mxchip-DN02QYHE', 'B':'mxchip-DN02QYI2', 'C':'mxchip-DN02QYI9', 'D':'mxchip-DN02QYIH', 'E':'mxchip-DN02QYJ4'}
device_list = list(devices)
device_attr={}
device_list.sort()
at=Autotest()
logname=time.strftime('%Y-%m-%d@%H-%M')
logname = 'line_topology-' + logname +'.log'
at.start('10.125.52.132', 34568, logname)
at.device_subscribe(devices)

for device in device_list:
    at.device_run_cmd(device, ['netmgr', 'clear'])
    at.device_run_cmd(device, ['kv', 'delete', 'alink'])
    ret = at.device_run_cmd(device, ['reboot'])
time.sleep(5)

#get device mac address
for device in device_list:
    at.device_run_cmd(device, ['umesh', 'extnetid', '122798010203'])
    at.device_run_cmd(device, ['umesh', 'stop'])
    mac = at.device_run_cmd(device, ['mac'], 1, 1, ['MAC address:'])
    if mac != False and mac != []:
        mac = mac[0].replace('MAC address: ', '')
        mac = mac.replace('-', '')
        device_attr[device] = {'mac':mac}
    else:
        print 'error: get mac addr for device {0} failed, ret={1}'.format(device, mac)
        exit(1)

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
        at.device_run_cmd(device, ['netmgr', 'connect', 'wuchen_test', 'aliyunos'])
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

    ipaddr = at.device_run_cmd(device, ['umesh', 'ipaddr'], 3, 1, [':'])
    if ipaddr == False or ipaddr == [] or len(ipaddr) != 3:
        print 'error: get ipaddr for device {0} failed'.format(device)
        exit(1)
    else:
        ipaddr[0] = ipaddr[0].replace('\t', '')
        ipaddr[1] = ipaddr[1].replace('\t', '')
        ipaddr[2] = ipaddr[2].replace('\t', '')
        device_attr[device]['ipaddr'] = ipaddr[0:3]
print "\n{0}\n".format(device_attr)

retry = 3
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
                        print '{0} ping {1} with {2} bytes by local ipv6 addr failed'.format(device, other, pkt_len)
                        fail_num += 1
                        break
                else:
                    success_num += 1
                    break

            dst_ip = device_attr[other]['ipaddr'][1]
            for i in range(retry):
                response = at.device_run_cmd(device, ['umesh', 'ping', dst_ip, pkt_len], 1, 1.5, filter)
                expected_response = '{0} bytes from {1}'.format(pkt_len, dst_ip)
                if response == False or response == [] or expected_response not in response[0]:
                    if i < retry - 1:
                        continue
                    else:
                        print '{0} ping {1} with {2} bytes by global ipv6 addr failed'.format(device, other, pkt_len)
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
                        print '{0} send {1} with {2} bytes by local ipv6 addr failed'.format(device, other, pkt_len)
                        fail_num += 1
                        break
                else:
                    success_num += 1
                    break

            dst_ip = device_attr[other]['ipaddr'][1]
            for i in range(retry):
                response = at.device_run_cmd(device, ['umesh', 'autotest', dst_ip, '1', pkt_len], 1, 1, filter)
                dst_ip = device_attr[other]['ipaddr'][0]
                expected_response = '{0} bytes autotest echo reply from {1}'.format(pkt_len, dst_ip)
                if response == False or response == [] or expected_response not in response[0]:
                    if i < retry - 1:
                        continue
                    else:
                        print '{0} send {1} with {2} bytes by global ipv6 addr failed'.format(device, other, pkt_len)
                        fail_num += 1
                        break
                else:
                    success_num += 1
                    break
print 'udp: succeed-{0}, failed-{1}'.format(success_num, fail_num)

at.stop()

