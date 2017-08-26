import os, sys, time
sys.path.append('../')
from autotest import Autotest

if len(sys.argv) < 2:
    print "use default ./default.bin to program".format(sys.argv[0])
    filename = './lb.bin'
else:
    filename = os.path.expanduser(sys.argv[1])

if os.path.isfile(filename) == False:
    print "error: file {0} does not exist".format(sys.argv[1])
    exit(1)

devices = {}
devices['00'] = 'mxchip-DN02QRIQ'
devices['01'] = 'mxchip-DN02QRIX'
devices['02'] = 'mxchip-DN02QRJ6'
devices['03'] = 'mxchip-DN02QRJ7'
devices['04'] = 'mxchip-DN02QRJ9'
devices['05'] = 'mxchip-DN02QRJE'
devices['06'] = 'mxchip-DN02QRJK'
devices['07'] = 'mxchip-DN02QRJM'
devices['08'] = 'mxchip-DN02QRJN'
devices['09'] = 'mxchip-DN02QRJP'
devices['00'] = 'mxchip-DN02QRJQ'
devices['11'] = 'mxchip-DN02QRJR'
devices['12'] = 'mxchip-DN02QRJU'
devices['13'] = 'mxchip-DN02QRJX'
devices['14'] = 'mxchip-DN02QRJY'
devices['15'] = 'mxchip-DN02QRK3'
devices['16'] = 'mxchip-DN02QRK6'
devices['17'] = 'mxchip-DN02QRK7'
devices['18'] = 'mxchip-DN02QRKF'
devices['19'] = 'mxchip-DN02QRKQ'
devices['10'] = 'mxchip-DN02X2ZO'
devices['21'] = 'mxchip-DN02X2ZS'
devices['22'] = 'mxchip-DN02X2ZU'
devices['23'] = 'mxchip-DN02X2ZZ'
devices['24'] = 'mxchip-DN02X304'
devices['25'] = 'mxchip-DN02X309'
devices['26'] = 'mxchip-DN02X30I'
devices['27'] = 'mxchip-DN02XLNN'

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

