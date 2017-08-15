import sys, time
sys.path.append('../')
from autotest import Autotest

devices = {'A':'mxchip-DN02QYHE', 'B':'mxchip-DN02QYI2', 'C':'mxchip-DN02QYI9'}
logname=time.strftime('%Y-%m-%d@%H-%M')
logname = 'example-' + logname +'.log'
at=Autotest()
at.start('10.125.52.132', 34568, logname)

at.device_subscribe(devices)
at.device_run_cmd('A', ['netmgr', 'clear'])
at.device_run_cmd('A', ['reboot'])
time.sleep(5)
at.device_run_cmd('A', ['netmgr', 'connect', 'wuchen_test', 'aliyunos'])
time.sleep(8)
filter = ['disabled', 'detached', 'attached', 'leaf', 'router', 'super_router', 'leader', 'unknown']
print at.device_run_cmd('A', ['umesh', 'state'], 1, 0.5, filter)

at.stop()

