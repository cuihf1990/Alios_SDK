import sys, os, time, httplib, json, subprocess, pdb
sys.path.append('../')
from autotest import Autotest

DEBUG = False
#server inteaction related functions
operations = {'status':'getCaseStatus', 'start': 'runCase', 'stop':'stopCase'}
statuscode = {'0':'idle', '1':'running', '2':'success', 3:'fail'}
def construct_url(operation, testid, auid):
    if operation not in list(operations):
        return ''
    if testid.isdigit() == False:
        return ''
    if auid.isdigit() == False:
        return ''
    url = '/' + operations[operation] + '?id=' + testid + '&auid=' + auid
    return url


def alink_test(conn, operation, testid, auid):
    headers = {'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
               'Accept-Encoding': 'gzip, deflate',
               'Accept-Language': 'en-us',
               'Connection': 'keep-alive',
               'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_6) AppleWebKit/603.3.8 (KHTML, like Gecko) Version/10.1.2 Safari/603.3.8'}
    url = construct_url(operation, testid, auid)
    if url == '':
        return {}

    try:
        conn.request('GET', url, '', headers)
    except:
        print 'error: connecting server to request service failed'
        return {}
    response = conn.getresponse()
    if response.status != 200:
        print 'http request error: retcode-{0}'.format(response.status)
        return {}
    respdata = response.read()
    try:
        return json.loads(respdata)
    except:
        if DEBUG:
            raise
        return {}

#main function
def main():
    global DEBUG
    testname = '5pps'
    device = 'mxchip-DN02QRKC'
    filename = 'lb.bin'
    caseid = '5366'
    userid = '500001007540092782'
    server = '10.125.11.56'
    wifissid = '112558'
    wifipass = 'test_wg@aliyun.com'
    port = '80'
    #parse input
    i = 1
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg.startswith('--testname='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --testname=5pps'.format(arg)
            testname = args[1]
        elif arg.startswith('--device='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --device=mxchip-DN02QRKC'.format(arg)
            device = args[1]
        elif arg.startswith('--filename='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --filename=firmware.bin'.format(arg)
            filename = args[1]
        elif arg.startswith('--caseid='):
            args = arg.split('=')
            if len(args) != 2 or args[1].isdigit() == False:
                print 'wrong argument {0} input, example: --caseid=12345'.format(arg)
            caseid = args[1]
        elif arg.startswith('--userid='):
            args = arg.split('=')
            if len(args) != 2 or args[1].isdigit() == False:
                print 'wrong argument {0} input, example: --userid=123456789012345678'.format(arg)
            userid = args[1]
        elif arg.startswith('--server='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --server=10.1.2.3'.format(arg)
            server = args[1]
        elif arg.startswith('--port='):
            args = arg.split('=')
            if len(args) != 2 or args[1].isdigit() == False:
                print 'wrong argument {0} input, example: --port=80'.format(arg)
            port = int(args[1])
        elif arg.startswith('--wifissid='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --wifissid=test_wifi'.format(arg)
            wifissid = args[1]
        elif arg.startswith('--wifipass='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --wifipass=test_password'.format(arg)
            wifipass = args[1]
        elif arg.startswith('--debug='):
            args = arg.split('=')
            if len(args) != 2 or args[1].isdigit() == False:
                print 'wrong argument {0} input, example: --debug=1'.format(arg)
            DEBUG = (args[1] != '0')
        elif arg=='--help':
            print 'Usage: python {0} [--testname=xxxx] [--device=xxx-xxx] [--filename=xxx.bin] [--caseid=xxx] [--userid=xxxxx] [--server=xx.x.x.x] [--port=xx] [--wifissid=wifi_ssid] [--wifipass=password] [--debug=0/1]'.format(sys.argv[0])
            sys.exit(0)
        i += 1

    logname=time.strftime('-%Y-%m-%d@%H-%M')
    logname = testname + logname +'.log'
    devices = {'A':device}
    device = 'A'

    #check test case status
    conn = httplib.HTTPConnection(server, port)
    result = alink_test(conn, 'status', caseid, userid)
    if DEBUG:
        print 'status:', result
    if result == {} or result[u'message'] != u'success':
        print 'error: unable to get test case {0} status'.format(caseid)
        sys.exit(1)
    if result[u'data'][u'case_status'] == 1:
        print 'error: test case {0} is already runing'.format(caseid)
        sys.exit(1)
    conn.close()

    at=Autotest()
    at.start('10.125.52.132', 34568, logname)
    if at.device_subscribe(devices) == False:
        print 'error: subscribe to device failed, some devices may not exist in testbed'
        sys.exit(1)

    #program device
    succeed = False; retry = 5
    print 'programming device {0} ...'.format(devices[device])
    for i in range(retry):
        if at.device_program(device, '0x13200', filename) == True:
            succeed = True
            break
    if succeed == False:
        print 'error: program device {0} failed'.format(devices[device])
        sys.exit(1)
    print 'program device {0} succeed'.format(devices[device])
    time.sleep(5)

    succeed = False; retry = 5
    while retry > 0:
        #clear previous setting and reboot
        at.device_run_cmd(device, ['kv', 'del', 'wifi'])
        at.device_run_cmd(device, ['kv', 'del', 'alink'])
        at.device_control(device, 'reset')
        time.sleep(5)

        #set a random mesh extnetid
        bytes = os.urandom(6)
        extnetid = ''
        for byte in bytes:
            extnetid = extnetid + '{0:02x}'.format(ord(byte))
        at.device_run_cmd(device, ['umesh', 'extnetid', extnetid])

        #connect device to alink
        at.device_run_cmd(device, ['netmgr', 'connect', wifissid, wifipass], timeout=1.5)
        time.sleep(15)
        filter = ['uuid:', 'alink is not connected']
        response = at.device_run_cmd(device, ['uuid'], 1, 1.5, filter)
        if response == False or len(response) != 1 or 'uuid:' not in response[0]:
            retry -= 1
            continue
        succeed = True
        break;
    if succeed == False:
        print 'error: connect device to alink failed, response = {0}'.format(response)
        sys.exit(1)

    #start run test case
    conn = httplib.HTTPConnection(server, port)
    result = alink_test(conn, 'start', caseid, userid)
    if DEBUG:
        print 'start:', result
    if result == {}:
        print 'error: unable to start test case {0}'.format(caseid)
        sys.exit(1)
    if result[u'message'] != u'success':
        print 'error: start test case {0} failed, return:{1}'.format(testid, result[u'message'])
        sys.exit(1)
    conn.close()
    time.sleep(5)


    #poll test case status
    while True:
        conn = httplib.HTTPConnection(server, port)
        result = alink_test(conn, 'status', caseid, userid)
        if DEBUG:
            print 'status:', result
        if result == {}:
            print 'error: unable to get test case {0} status'.format(caseid)
            sys.exit(1)
        if result[u'message'] != u'success' or result[u'data'][u'case_status'] != 1:
            break;
        conn.close()
        time.sleep(120)


    #print result
    try:
        print result[u'data'][u'case_fail_desc'].encode('utf-8')
    except:
        pass

    if result[u'data'][u'case_status'] != 2:
        print 'test {0} finished unsuccessfully'.format(testname)
        sys.exit(1)
    else:
        print 'test {0} finished successfully'.format(testname)
        sys.exit(0)

if __name__ == '__main__':
    main()
