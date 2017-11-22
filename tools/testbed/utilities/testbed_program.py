import os, sys, time
from autotest import Autotest

if len(sys.argv) < 2:
    print "use default master.bin to program".format(sys.argv[0])
    filename = 'master.bin'
else:
    filename = os.path.expanduser(sys.argv[1])

if os.path.isfile(filename) == False:
    print "error: file {0} does not exist".format(filename)
    exit(1)

devices = {}
devices['00'] = 'mxchip-DN02QRIQ'
devices['01'] = 'mxchip-DN02QRIU'
devices['02'] = 'mxchip-DN02QRIX'
devices['03'] = 'mxchip-DN02QRJ3'
devices['04'] = 'mxchip-DN02QRJ6'
devices['05'] = 'mxchip-DN02QRJ7'
devices['06'] = 'mxchip-DN02QRJ8'
#devices['07'] = 'mxchip-DN02QRJ9'
#devices['08'] = 'mxchip-DN02QRJE'
#devices['09'] = 'mxchip-DN02QRJK'
#devices['10'] = 'mxchip-DN02QRJL'
#devices['11'] = 'mxchip-DN02QRJM'
#devices['12'] = 'mxchip-DN02QRJN'
#devices['13'] = 'mxchip-DN02QRJP'
#devices['14'] = 'mxchip-DN02QRJQ'
#devices['15'] = 'mxchip-DN02QRJR'
#devices['16'] = 'mxchip-DN02QRJU'
#devices['17'] = 'mxchip-DN02QRJX'
#devices['18'] = 'mxchip-DN02QRJY'
#devices['19'] = 'mxchip-DN02QRK3'
#devices['20'] = 'mxchip-DN02QRK6'
#devices['21'] = 'mxchip-DN02QRK7'
#devices['22'] = 'mxchip-DN02QRKB'
#devices['23'] = 'mxchip-DN02QRKE'
#devices['24'] = 'mxchip-DN02QRKM'
#devices['25'] = 'mxchip-DN02QRKQ'
#devices['26'] = 'mxchip-DN02QYHW'
#devices['27'] = 'mxchip-DN02RDVL'
#devices['28'] = 'mxchip-DN02RDVT'
#devices['29'] = 'mxchip-DN02RDVV'
#devices['30'] = 'mxchip-DN02X2ZX'
#devices['31'] = 'mxchip-DN02X2ZZ'
#devices['32'] = 'mxchip-DN02X303'
#devices['33'] = 'mxchip-DN02X309'
#devices['34'] = 'mxchip-DN02X30B'
#devices['35'] = 'mxchip-DN02X30H'
#devices['36'] = 'mxchip-DN02X2ZO'
#devices['37'] = 'mxchip-DN02X2ZS'
#devices['38'] = 'mxchip-DN02X304'

device_list = list(devices)
device_attr={}
device_list.sort()
at=Autotest()
logname=time.strftime('%Y-%m-%d@%H-%M')
logname = 'testbed_program-' + logname +'.log'
at.start('10.125.52.132', 34568, logname)

#reboot and get device mac address
success_num = 0; fail_num = 0
for device in device_list:
    if at.device_subscribe({device:devices[device]}) == False:
        print 'programming {0}:{1} error: devices not connected'.format(device, devices[device])
        fail_num += 1
        continue
    if at.device_program(device, '0x13200', filename) == False:
        print 'programming {0}:{1} failed'.format(device, devices[device])
        fail_num += 1
    else:
        print 'programming {0}:{1} succeed'.format(device, devices[device])
        success_num += 1
print 'programming result: succeed-{0}, failed-{1}'.format(success_num, fail_num)

at.stop()

