import sys, time
sys.path.append("../")
from autotest import Autotest

#  devices = {"A":"mxchip-DN02QYH3", "B":"mxchip-DN02QYHF", "C":"mxchip-DN02QYHK", "D":"mxchip-DN02QYHO", "E":"mxchip-DN02QYIJ"}
devices = {"A":"mxchip-DN02QYH3", "B":"mxchip-DN02QYHF", "C":"mxchip-DN02QYHK", "D":"mxchip-DN02QYHO"}
device_list = list(devices)
device_list.sort()
device_mac={}
at=Autotest()
at.start("10.125.52.132", 34568, "line_topology_test.log")
at.device_subscribe(devices)

for device in device_list:
    at.device_run_cmd(device, ['netmgr', 'clear'])
    at.device_run_cmd(device, ['kv', 'delete', 'alink'])
    ret = at.device_run_cmd(device, ['reboot'])
time.sleep(3)

for device in device_list:
    at.device_run_cmd(device, ['umesh', 'stop'])
    mac = at.device_run_cmd(device, ['mac'], 1, 0.8)
    if mac != []:
        mac = mac[0].replace("MAC address: ", "")
        mac = mac.replace("-", "")
        device_mac[device] = mac
    else:
        print "error: get mac addr for device {0} failed".format(device)
        exit(1)
print device_mac

#setup line topology
for i in range(len(device_list)):
    device = device_list[i]
    at.device_run_cmd(device, ["umesh", "whitelist", "clear"])
    if (i-1) > 0:
        prev_dev = device_list[i-1]
        at.device_run_cmd(device, ["umesh", "whitelist", "add", device_mac[prev_dev]+"0000"])
    if (i+1) < len(device_list):
        next_dev = device_list[i+1]
        at.device_run_cmd(device, ["umesh", "whitelist", "add", device_mac[next_dev]+"0000"])
    at.device_run_cmd(device, ["umesh", "whitelist", "enable"])
    at.device_run_cmd(device, ["umesh", "whitelist"])

#start gateway leader
at.device_run_cmd("A", ["netmgr", "connect", "Alibaba_test_wb065324TP_LINK", "aliyunos_1234"])
time.sleep(8)
uuid = at.device_run_cmd("A", ["uuid"], 1, 0.8)
if uuid == []:
    print "error: alink connect to server failed"
    exit(1)
state = at.device_run_cmd("A", ["umesh", "state"], 1, 0.8)
if state != ["leader"]:
    print "error: mesh start failed"

at.stop()

