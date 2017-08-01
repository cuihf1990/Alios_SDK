import sys, time
sys.path.append("../")
from autotest import Autotest

devices = {'A':"mxchip-DN02QYH3", "B":"mxchip-DN02QYHF", "C":"mxchip-DN02QYHK"}
at=Autotest()
at.start("10.125.52.132", 34568, "example_test.log")

at.device_subscribe(devices)
at.device_run_cmd('A', ['netmgr', 'clear'])
at.device_run_cmd('A', ['reboot'])
time.sleep(2)
at.device_run_cmd('A', ['netmgr', 'connect', 'Alibaba_test_wb065324TP_LINK', 'aliyunos_1234'])
time.sleep(8)
print at.device_run_cmd('A', ['umesh', 'state'], 1, 0.5)

at.stop()

