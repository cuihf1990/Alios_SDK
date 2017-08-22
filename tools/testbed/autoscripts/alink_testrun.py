import sys, time, httplib, json, pdb

DEBUG = False
server = '30.6.61.154'
port = 8080
operations = {'status':'getCaseStatus', 'start': 'runCase', 'stop':'stopCase'}
caseid = '5341'
userid = '500001013300203877'
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
        print "error: connecting server to request service failed"
        return {}
    response = conn.getresponse()
    if response.status != 200:
        print "http request error: retcode-{0}".format(response.status)
        return {}
    respdata = response.read()
    try:
        return json.loads(respdata)
    except:
        if DEBUG:
            raise
        return {}

i = 1
while i < len(sys.argv):
    arg = sys.argv[i]
    if arg.startswith('--caseid='):
        args = arg.split('=')
        if len(args) != 2 or args[1].isdigit() == False:
            print "wrong argument {0} input, example: --caseid=12345".format(arg)
        caseid = args[1]
    elif arg.startswith('--userid='):
        args = arg.split('=')
        if len(args) != 2 or args[1].isdigit() == False:
            print "wrong argument {0} input, example: --userid=123456789012345678".format(arg)
        userid = args[1]
    elif arg.startswith('--server='):
        args = arg.split('=')
        if len(args) != 2:
            print "wrong argument {0} input, example: --server=10.1.2.3".format(arg)
        server = args[1]
    elif arg.startswith('--port='):
        args = arg.split('=')
        if len(args) != 2 or args[1].isdigit() == False:
            print "wrong argument {0} input, example: --port=80".format(arg)
        port = int(args[1])
    elif arg.startswith('--debug='):
        args = arg.split('=')
        if len(args) != 2 or args[1].isdigit() == False:
            print "wrong argument {0} input, example: --debug=1".format(arg)
        DEBUG = (args[1] != '0')
    elif arg=='--help':
        print "Usage: python {0} [--caseid=xxx] [--userid=xxxxx] [--server=10.1.2.3] [--port=80] [--debug=1]".format(sys.argv[0])
        sys.exit(0)
    i += 1

conn = httplib.HTTPConnection(server, port)

#check test case status
result = alink_test(conn, 'status', caseid, userid)
if DEBUG:
    print "status:", result
if result == {} or result[u'message'] != u'success':
    print "error: unable to get test case {0} status".format(caseid)
    sys.exit(1)
if result[u'data'][u'case_status'] == 1:
    print "error: test case {0} is already runing".format(caseid)
    sys.exit(1)

#start run test case
result = alink_test(conn, 'start', caseid, userid)
if DEBUG:
    print "start:", result
if result == {}:
    print "error: unable to start test case {0}".format(caseid)
    sys.exit(1)
if result[u'message'] != u'success':
    print "error: start test case {0} failed, return:{1}".format(testid, result[u'message'])
    sys.exit(1)
time.sleep(5)


#poll test case status
while True:
    result = alink_test(conn, 'status', caseid, userid)
    if DEBUG:
        print "status:", result
    if result == {}:
        print "error: unable to get test case {0} status".format(caseid)
        sys.exit(1)
    if result[u'message'] != u'success' or result[u'data'][u'case_status'] != 1:
        break;
    time.sleep(120)

conn.close()

#print result
if result[u'data'][u'case_status'] != 2:
    print result[u'data'][u'case_fail_desc']
    print "test case {0} finished unsuccessfully".format(caseid)
    sys.exit(1)
else:
    print "test case {0} finished successfully".format(caseid)
    sys.exit(0)

