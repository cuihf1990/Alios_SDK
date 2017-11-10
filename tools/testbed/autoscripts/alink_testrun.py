import sys, os, time, httplib, json, subprocess, pdb
from autotest import Autotest

models={'mk3060':'0x13200', 'esp32':'0x10000'}
testnames={'5pps':0, '2pps':1, 'short5pps':15, 'short2pps':16}

caseids={}
#mk3060
caseids['39F841C8CE86C0B5F3FC864925EB1702'] = 26672 #DN02QRK7
caseids['1B12DA63F1E56C73CE237A495E8C5087'] = 27725 #DN02QRKB
caseids['82A4C68BA791767250242D4D83594EF3'] = 27419 #DN02QRKQ
caseids['F384E85B01EF16120033B21833C7F0D4'] = 27436 #DN02RDVL
caseids['E57E046D3A6F91257E2551710E6FED23'] = 27453 #DN02RDVT
caseids['A9036FCE11259562AC9C90999DA77F15'] = 27470 #DN02RDVV
caseids['E2D2A7AF74761FC8DF6FD423D6F9B5A4'] = 26621 #DN02X2ZO
caseids['236838BD263486D6A0CEA77FB3D8110E'] = 26604 #DN02X2ZS
caseids['FFBD93C07D8286489A34293D2E691A7E'] = 27487 #DN02X2ZX
caseids['40BB9203D70AA8628E93FB6379DD628B'] = 26570 #DN02X2ZZ
caseids['7089FD4690110E8194F29CDCBC82BC0D'] = 26536 #DN02X303
caseids['536C0EB02B06B21AB6F54333ED18415F'] = 26553 #DN02X304
caseids['3BD14E6396E3074B61CB83F9257A4F01'] = 27504 #DN02X30B
caseids['47112F8E0A5CDFBE05B570F4C7306801'] = 26587 #DN02X30H
#esp32
caseids['7D917B0B8CF687EC496B3B7788CE6186'] = 27521 #espif-1.1
caseids['F30F0804BC3752D2E6F43EF35DBE8E29'] = 27538 #espif-1.2.2
caseids['F731214AA77B4134FD988BC89E7B7B00'] = 27555 #espif-1.2.3
caseids['59ED5E3E36FF2BDCB81B0FBCE9E997BF'] = 27572 #espif-1.3
caseids['0CD754231D66D79A38CBD8416F37A4E8'] = 27589 #espif-1.4
caseids['41DB75B637872CFEB194C79476568F59'] = 27606 #espif-2.1
caseids['A4FFC78630480DB8768D04091B282D54'] = 27623 #espif-2.2.1
caseids['938F2BFC13AE8F6C4098289BB2D02B93'] = 27640 #espif-2.2.2
caseids['FD1FC11D9B9F6511FBD6E86168822072'] = 27657 #espif-2.2.3
caseids['D5D8715DA640D401D3836A3C854C4021'] = 27674 #espif-2.3
caseids['A007D2D3AE28C4B501A7D57D93B253B4'] = 27044 #espif-2.4
caseids['A2A268CC74FEB91744C68E652AE89266'] = 27691 #espif-9.1
caseids['9EA130B3F97FEDD4A40B58E223B0EA23'] = 27708 #espif-9.2.1
caseids['785A61C6F5656AD16D57A919F0CEDFB4'] = 27015 #espif-9.2.2
caseids['2363A965CE3C548993F485EF5883F6CA'] = 26689 #espif-9.2.3
caseids['1A2D6B99CF19A1DE70E197798F2D68FE'] = 27061 #espif-9.3
caseids['33688B9B824E66254D109CDAC1C66CF4'] = 26998 #espif-9.4
caseids['794736BDFF2E591F6BFEC2FE3EE383E6'] = 26638 #espif-x
caseids['E55F17709F0A11180D36AE83720EC22B'] = 27078 #espif-x

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
def main(firmware='~/lb-all.bin', model='mk3060', testname='5pps'):
    global DEBUG
    userid = '500001169232518525'
    server = 'pre-iotx-qs.alibaba.com'
    port = '80'
    wifissid = 'aos_test_01'
    wifipass = 'Alios@Embedded'
    testbed_server = '10.125.52.132'

    #parse input
    i = 1
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg.startswith('--firmware='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --firmware=firmware.bin'.format(arg)
            firmware = args[1]
        elif arg.startswith('--model='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --model=mk3060'.format(arg)
            model = args[1]
        elif arg.startswith('--testname='):
            args = arg.split('=')
            if len(args) != 2:
                print 'wrong argument {0} input, example: --testname=5pps'.format(arg)
            testname = args[1]
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
            print 'Usage: python {0} [--firmware=xxx.bin] [--model=xxx] [--testname=xxxx] [--userid=xxxxx] [--server=xx.x.x.x] [--port=xx] [--wifissid=wifi_ssid] [--wifipass=password] [--debug=0/1]'.format(sys.argv[0])
            return [0, 'help']
        i += 1

    if DEBUG:
        print "firmware: {0}".format(firmware)
        print "model: {0}".format(model)
        print "testname: {0}".format(testname)
        print "userid: {0}".format(userid)
        print "server: {0}".format(server)
        print "port: {0}".format(port)
        print "wifissid: {0}".format(wifissid)
        print "wifipass: {0}".format(wifipass)

    if testname not in testnames:
        print "error: unsupported testname {0}".format(repr(testname))
        return [1, 'testname {0} unsupported'.format(repr(testname))]

    if not model or model.lower() not in models:
        print "error: unsupported model {0}".format(repr(model))
        return [1, 'model {0} unsupported'.format(repr(model))]
    model = model.lower()

    logname=time.strftime('-%Y-%m-%d@%H-%M')
    logname = testname + logname +'.log'
    at=Autotest()
    if at.start(testbed_server, 34568, logname) == False:
        print 'error: start failed'
        return [1, 'connect testbed failed']

    number = 1
    if testname in ['5pps', '2pps']:
        timeout = 3600
    else:
        timeout = 120
    allocted = at.device_allocate(model, number, timeout, 'alink')
    if len(allocted) != number:
        print "error: request device allocation failed"
        return [1, 'allocate device failed']
    print "allocted device", allocted[0]

    devices = {'A':allocted[0]}
    device = 'A'

    #subscribe device
    if at.device_subscribe(devices) == False:
        print 'error: subscribe to device failed, some devices may not exist in testbed'
        return [1, 'subscribe device failed']

    #program device
    succeed = False; retry = 5
    addr = models[model]
    print 'programming device {0} ...'.format(devices[device])
    for i in range(retry):
        if at.device_program(device, addr, firmware) == True:
            succeed = True
            break
        time.sleep(0.5)
    if succeed == False:
        print 'error: program device {0} failed'.format(devices[device])
        return [1, 'program device failed']
    print 'program device {0} succeed'.format(devices[device])
    time.sleep(5)

    #connect to alink
    succeed = False; retry = 5
    uuid = None
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
        uuid = response[0].split()[-1]
        print "connect alink succeed, uuid: {0}".format(uuid)
        succeed = True
        break;
    if succeed == False:
        print 'error: connect device to alink failed, response = {0}'.format(response)
        return [1, 'connect alink failed']
    if uuid not in caseids:
        print 'error: device uuid {0} not in supported list'.format(uuid)
        return [1, 'uuid {0} invalid'.format(uuid)]
    caseid = caseids[uuid] + testnames[testname]
    caseid = str(caseid)
    if DEBUG:
        print "caseid: {0}".format(caseid)

    #check test case status
    already_running = False
    conn = httplib.HTTPConnection(server, port)
    result = alink_test(conn, 'status', caseid, userid)
    if DEBUG:
        print 'status:', result
    if result == {} or result[u'message'] != u'success':
        print 'error: unable to get test case {0} status'.format(caseid)
        return [1, 'get case {0} status failed'.format(caseid)]
    if result[u'data'][u'case_status'] == 1:
        print 'test case {0} is already runing'.format(caseid)
        already_running = True
    conn.close()

    if already_running:
        #already running, stop test case
        conn = httplib.HTTPConnection(server, port)
        result = alink_test(conn, 'stop', caseid, userid)
        if DEBUG:
            print 'status:', result
        if result == {} or result[u'message'] != u'success':
            print 'error: unable to stop test case {0}'.format(caseid)
            return [1, 'stop case {0} failed'.format(caseid)]
        conn.close()
        print 'stop case {0} succeed'.format(caseid)

    #start run test case
    conn = httplib.HTTPConnection(server, port)
    result = alink_test(conn, 'start', caseid, userid)
    if DEBUG:
        print 'start:', result
    if result == {}:
        print 'error: unable to start test case {0}'.format(caseid)
        return [1, 'start case failed']
    if result[u'message'] != u'success':
        print 'error: start test case {0} failed, return:{1}'.format(testid, result[u'message'])
        return [1, 'start case failed']
    conn.close()
    time.sleep(5)

    #poll test case status
    retry = 5
    while retry > 0:
        try:
            conn = httplib.HTTPConnection(server, port)
            result = alink_test(conn, 'status', caseid, userid)
            if DEBUG:
                print 'status:', result
            if result == {}:
                print 'error: unable to get test case {0} status'.format(caseid)
                return [1, 'get status failed']
            if result[u'message'] != u'success' or result[u'data'][u'case_status'] != 1:
                break;
            conn.close()
        except:
            retry -= 1
        time.sleep(120)


    #print result
    try:
        print result[u'data'][u'case_fail_desc'].encode('utf-8')
    except:
        pass

    if result[u'data'][u'case_status'] != 2:
        print 'test {0} finished unsuccessfully'.format(testname)
        return [1, 'failed']
    else:
        print 'test {0} finished successfully'.format(testname)
        return [0, 'succeed']

if __name__ == '__main__':
    [code, msg] = main()
    sys.exit(code)
