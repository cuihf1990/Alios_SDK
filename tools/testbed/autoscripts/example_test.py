import sys, time
sys.path.append('../')
from autotest import Autotest

devices = {'A':'mxchip-DN02QRIQ', 'B':'mxchip-DN02QRIX', 'C':'mxchip-DN02QRJ6'}
logname=time.strftime('%Y-%m-%d@%H-%M')
logname = 'example-' + logname +'.log'
at=Autotest()
at.start('10.125.52.132', 34568, logname)
if at.device_subscribe(devices) == False:
    print 'error: subscribe to device failed, some devices may not exist in testbed'
    exit(1)

at.device_run_cmd('A', ['netmgr', 'clear'])
at.device_run_cmd('A', ['reboot'])
time.sleep(5)
at.device_run_cmd('A', ['netmgr', 'connect', 'wuchen_test', 'aliyunos'])
time.sleep(10)
filter = ['disabled', 'detached', 'attached', 'leaf', 'router', 'super_router', 'leader', 'unknown']
print at.device_run_cmd('A', ['umesh', 'state'], 1, 0.5, filter)

at.stop()

