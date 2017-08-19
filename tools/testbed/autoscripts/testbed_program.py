import os, sys, time
sys.path.append('../')
from autotest import Autotest

if len(sys.argv) != 2:
    print "usage: {0} filename".format(sys.argv[0])
    exit(0)

filename = os.path.expanduser(sys.argv[1])
if os.path.isfile(filename) == False:
    print "error: file {0} does not exist".format(sys.argv[1])
    exit(1)

devices = {}
devices['00'] = 'mxchip-DN02QRIX'
devices['01'] = 'mxchip-DN02QRJ9'
devices['02'] = 'mxchip-DN02QRJK'
devices['03'] = 'mxchip-DN02QRJM'
devices['04'] = 'mxchip-DN02QRJP'
devices['05'] = 'mxchip-DN02QRK3'
devices['06'] = 'mxchip-DN02X2ZT'
devices['07'] = 'mxchip-DN02X2ZZ'

devices['08'] = 'mxchip-DN02QRJ3'
devices['09'] = 'mxchip-DN02QRJN'
devices['00'] = 'mxchip-DN02QRKB'
devices['11'] = 'mxchip-DN02QRKE'
devices['12'] = 'mxchip-DN02QRKM'
devices['13'] = 'mxchip-DN02RDVV'
devices['14'] = 'mxchip-DN02X300'
devices['15'] = 'mxchip-DN02X309'
devices['16'] = 'mxchip-DN02X30H'

devices['17'] = 'mxchip-DN02QRJD'
devices['18'] = 'mxchip-DN02QRJH'
devices['19'] = 'mxchip-DN02QRJL'
devices['10'] = 'mxchip-DN02QRJS'
devices['21'] = 'mxchip-DN02QRJU'
devices['22'] = 'mxchip-DN02QRK1'
devices['23'] = 'mxchip-DN02RDVL'
devices['24'] = 'mxchip-DN02X305'

device_list = list(devices)
device_attr={}
device_list.sort()
at=Autotest()
logname=time.strftime('%Y-%m-%d@%H-%M')
logname = 'testbed_program-' + logname +'.log'
at.start('10.125.52.132', 34568, logname)
if at.device_subscribe(devices) == False:
    print 'error: subscribe to device failed, some devices may not exist in testbed'
    exit(1)

#reboot and get device mac address
success_num = 0; fail_num = 0
for device in device_list:
    if at.device_program(device, '0x13200', filename) == False:
        print 'programming {0}:{1} failed'.format(device, devices[device])
        fail_num += 1
    else:
        print 'programming {0}:{1} succeed'.format(device, devices[device])
        success_num += 1
print 'programming result: succeed-{0}, failed-{1}'.format(success_num, fail_num)

at.stop()

