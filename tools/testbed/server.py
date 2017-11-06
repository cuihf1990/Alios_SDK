import os, sys, time, socket
import thread, threading, json
import TBframe

MAX_MSG_LENTH      = 2000
DEBUG = False

class Server:
    def __init__(self):
        self.client_socket = 0
        self.terminal_socket = 0
        self.client_list = []
        self.terminal_list = []
        self.allocated = {'lock':threading.Lock(), 'devices':[], 'timeout':0}
        self.keep_running = True

    def construct_dev_list(self):
        l = []
        for client in self.client_list:
            devices = client['addr'][0]
            devices += ','+ str(client['addr'][1])
            for port in client['devices']:
                devices += ',' + port + '|' + str(client['devices'][port]['using'])
            l.append(devices)
        data = ':'.join(l)
        return data

    def send_device_list_to_terminal(self, terminal):
        devs = self.construct_dev_list()
        data = TBframe.construct(TBframe.ALL_DEV, devs)
        terminal['socket'].send(data)

    def send_device_list_to_all(self):
        devs = self.construct_dev_list()
        data = TBframe.construct(TBframe.ALL_DEV, devs)
        for t in self.terminal_list:
            try:
                t['socket'].send(data)
            except:
                continue

    def client_serve_thread(self, client):
        client['socket'].settimeout(1)
        heartbeat_timeout = time.time() + 30
        file = {}
        msg = ''
        while self.keep_running:
            try:
                if time.time() > heartbeat_timeout:
                    print "client {0} heartbeat timeout".format(client['addr'])
                    break

                new_msg = client['socket'].recv(MAX_MSG_LENTH)
                if new_msg == '':
                    break

                msg += new_msg
                while msg != '':
                    type, length, value, msg = TBframe.parse(msg)
                    if type == TBframe.TYPE_NONE:
                        break

                    heartbeat_timeout = time.time() + 30
                    if type == TBframe.CLIENT_DEV:
                        new_devices = value.split(':')
                        for port in new_devices:
                            if port != "" and port not in client['devices']:
                                print "device {0} added to client {1}".format(port, client['addr'])
                                client['devices'][port] = {
                                        'lock':threading.Lock(),
                                        'using':0,
                                        'status':'{}',
                                        'log_subscribe':[],
                                        'status_subscribe':[]
                                        }

                        for port in list(client['devices']):
                            if port not in new_devices:
                                print "device {0} removed from client {1}".format(port, client['addr'])
                                client['devices'].pop(port)

                        for port in list(client['devices']):
                            if port !="" and port not in file:
                                try:
                                    filename = 'server/' + client['addr'][0] + '-' + port[5:] + '.log'
                                    file[port] = open(filename, 'a')
                                except:
                                    print "error: can not open/create file ", filename
                                    continue
                        for f in list(file):
                            if f not in client['devices']:
                                file[f].close()
                                file.pop(f)
                        self.send_device_list_to_all()
                    elif type == TBframe.DEVICE_LOG:
                        port = value.split(':')[0]
                        if port not in file:
                            continue

                        try:
                            logtime = value.split(':')[1]
                            logstr = value[len(port) + 1 + len(logtime):]
                            logtime = float(logtime)
                            logtimestr=time.strftime("%Y-%m-%d@%H:%M:%S", time.localtime(logtime))
                            logtimestr += ("{0:.3f}".format(logtime-int(logtime)))[1:]
                            logstr = logtimestr + logstr
                            file[port].write(logstr)
                        except:
                            if DEBUG:
                                raise
                            continue
                        if 'tag' in client and client['tag'] in logstr:
                            continue
                        if client['devices'][port]['log_subscribe'] != []:
                            log = client['addr'][0] + ',' + str(client['addr'][1]) + ',' + port
                            log += value[len(port):]
                            data = TBframe.construct(type, log)
                            for s in client['devices'][port]['log_subscribe']:
                                try:
                                    s.send(data)
                                except:
                                    continue
                    elif type == TBframe.DEVICE_STATUS:
                        #print value
                        port = value.split(':')[0]
                        if port not in client['devices']:
                            continue
                        client['devices'][port]['status'] = value[len(port)+1:]
                        if client['devices'][port]['status_subscribe'] != []:
                            log = client['addr'][0] + ',' + str(client['addr'][1]) + ',' + port
                            log += value[len(port):]
                            data = TBframe.construct(type, log)
                            for s in client['devices'][port]['status_subscribe']:
                                try:
                                    s.send(data)
                                except:
                                    continue
                    elif type == TBframe.DEVICE_ERASE or type == TBframe.DEVICE_PROGRAM or \
                         type == TBframe.DEVICE_START or type == TBframe.DEVICE_STOP or \
                         type == TBframe.DEVICE_RESET or type == TBframe.DEVICE_CMD or \
                         type == TBframe.FILE_BEGIN or type == TBframe.FILE_DATA or \
                         type == TBframe.FILE_END:
                        values = value.split(',')
                        addr = (values[0], int(values[1]))
                        terminal = ''
                        for t in self.terminal_list:
                            if t['addr'] == addr:
                                terminal = t
                        if terminal != '':
                            if values[2] != 'success' and values[2] != 'ok':
                                data = TBframe.construct(TBframe.CMD_ERROR, ','.join(values[2:]))
                            else:
                                data = TBframe.construct(TBframe.CMD_DONE, ','.join(values[2:]))
                            terminal['socket'].send(data)
                    elif type == TBframe.CLIENT_TAG:
                        client['tag'] = value
                        print 'tag=', repr(value)
            except socket.timeout:
                continue
            except:
                if DEBUG:
                    raise
                break
        client['socket'].close()
        print "client ", client['addr'], "disconnected"
        self.client_list.remove(client)
        self.send_device_list_to_all()

    def send_file_to_someone(self, dst, filename):
        if os.path.exists(filename) == False:
            print "{0} does not exist\n".format(filename)
            return False
        print "sending {0} to {1} ...".format(filename, dst['addr']),
        file = open(filename,'r')
        content = filename.split('/')[-1]
        data = TBframe.construct(TBframe.FILE_BEGIN, content)
        try:
            dst['socket'].send(data)
        except:
            print "failed"
            return False
        content = file.read(1024)
        while(content):
            data = TBframe.construct(TBframe.FILE_DATA, content)
            try:
                dst['socket'].send(data)
            except:
                print "failed"
                return False
            content = file.read(1024)
        file.close()
        content = filename.split('/')[-1]
        data = TBframe.construct(TBframe.FILE_END, content)
        try:
            dst['socket'].send(data)
        except:
            print "failed"
            return False
        print "succeed"
        return True

    def get_client_by_addr(self, addr):
        ret = None
        for client in self.client_list:
            if client['addr'] != addr:
                continue
            ret = client
            break
        return ret

    def allocate_devices(self, value):
        values = value.split(',')
        if len(values) != 2:
            return ['error','argument']

        type = values[0]
        number = values[1]
        try:
            number = int(number)
        except:
            return ['error','argument']

        if number <= 0:
            return ['error','argument']

        allocated = []
        with self.allocated['lock']:
            for client in self.client_list:
                allocated = []
                ports = list(client['devices'])
                ports.sort()
                for port in ports:
                    if port in self.allocated['devices']:
                        continue

                    try:
                        status = json.loads(client['devices'][port]['status'])
                    except:
                        print 'parse {0} status failed'.format(port)
                        status = None
                    if status:
                        if 'model' not in status or type.lower() != status['model'].lower():
                            continue
                    else:
                        pathstr = {'mk3060':'mxchip', 'esp32':'espif'}[type.lower()]
                        if pathstr not in port:
                            continue

                    if client['devices'][port]['using'] != 0:
                        continue
                    allocated.append(port)
                    if len(allocated) >= number:
                        break
                if len(allocated) >= number:
                    break
            if len(allocated) >= number:
                self.allocated['devices'] += allocated
                self.allocated['timeout'] = time.time() + 10
                return ['success', '|'.join(allocated)]
            else:
                return ['fail', 'busy']

    def increase_device_refer(self, client, port, using_list):
        if [client['addr'], port] not in using_list:
            if port in list(client['devices']):
                with client['devices'][port]['lock']:
                    client['devices'][port]['using'] += 1
                using_list.append([client['addr'], port])
                self.send_device_list_to_all();

    def terminal_serve_thread(self, terminal):
        self.send_device_list_to_terminal(terminal)
        using_list = []
        terminal['socket'].settimeout(1)
        heartbeat_timeout = time.time() + 30
        msg = ''
        while self.keep_running:
            try:
                if time.time() > heartbeat_timeout:
                    print "terminal {0} heartbeat timeout".format(terminal['addr'])
                    break

                new_msg = terminal['socket'].recv(MAX_MSG_LENTH);
                if new_msg == '':
                    break

                msg += new_msg
                while msg != '':
                    type, length, value, msg = TBframe.parse(msg)
                    if type == TBframe.TYPE_NONE:
                        break

                    heartbeat_timeout = time.time() + 30
                    if type == TBframe.FILE_BEGIN or type == TBframe.FILE_DATA or type == TBframe.FILE_END:
                        target = value.split(':')[0]
                        target_data = value[len(target):]
                        [ip, port] = target.split(',')[0:2]
                        addr = (ip, int(port))
                        client = self.get_client_by_addr(addr)
                        if client == None:
                            data = TBframe.construct(TBframe.CMD_ERROR, 'noexist')
                            terminal['socket'].send(data)
                            continue
                        content = terminal['addr'][0]
                        content += ',' + str(terminal['addr'][1])
                        content += target_data
                        data = TBframe.construct(type, content)
                        client['socket'].send(data)
                    elif type == TBframe.DEVICE_ERASE:
                        dst = value.split(',')
                        addr = (dst[0], int(dst[1]))
                        client = self.get_client_by_addr(addr)
                        if client == None:
                            data = TBframe.construct(TBframe.CMD_ERROR,'fail')
                            terminal['socket'].send(data)
                        else:
                            content = terminal['addr'][0]
                            content += ',' + str(terminal['addr'][1])
                            content += ',' + dst[2]
                            data = TBframe.construct(TBframe.DEVICE_ERASE, content)
                            client['socket'].send(data)
                            self.increase_device_refer(client, dst[2], using_list)
                    elif type == TBframe.DEVICE_PROGRAM:
                        dst = value.split(',')
                        addr = (dst[0], int(dst[1]))
                        client = self.get_client_by_addr(addr)
                        if client == None:
                            data = TBframe.construct(TBframe.CMD_ERROR,'fail')
                            terminal['socket'].send(data)
                        else:
                            content = terminal['addr'][0]
                            content += ',' + str(terminal['addr'][1])
                            content += ',' + dst[2]
                            content += ',' + dst[3]
                            content += ',' + dst[4]
                            data = TBframe.construct(TBframe.DEVICE_PROGRAM, content)
                            client['socket'].send(data)
                            self.increase_device_refer(client, dst[2], using_list)
                    elif type == TBframe.DEVICE_RESET or type == TBframe.DEVICE_START or type == TBframe.DEVICE_STOP:
                        dst = (value.split(':')[0]).split(',')
                        addr = (dst[0], int(dst[1]))
                        client = self.get_client_by_addr(addr)
                        if client != None:
                            content = terminal['addr'][0]
                            content += ',' + str(terminal['addr'][1])
                            content += ',' + dst[2]
                            data = TBframe.construct(type, content)
                            client['socket'].send(data)
                            self.increase_device_refer(client, dst[2], using_list)
                    elif type == TBframe.DEVICE_CMD:
                        dst = (value.split(':')[0]).split(',')
                        devlen = len(value.split(':')[0])
                        cmds = value[devlen:]
                        addr = (dst[0], int(dst[1]))
                        client = self.get_client_by_addr(addr)
                        if client != None:
                            content = terminal['addr'][0]
                            content += ',' + str(terminal['addr'][1])
                            content += ',' + dst[2] + cmds
                            data = TBframe.construct(TBframe.DEVICE_CMD, content)
                            client['socket'].send(data)
                            self.increase_device_refer(client, dst[2], using_list)
                        else:
                            data = TBframe.construct(TBframe.CMD_ERROR,'fail')
                            terminal['socket'].send(data)
                    elif type == TBframe.DEVICE_ALLOC:
                        result = self.allocate_devices(value)
                        content = ','.join(result)
                        data = TBframe.construct(TBframe.DEVICE_ALLOC, content)
                        terminal['socket'].send(data)
                    elif type == TBframe.LOG_SUB:
                        values = value.split(',')
                        if len(values) != 3:
                            continue
                        addr = (values[0], int(values[1]))
                        port = values[2]
                        client = self.get_client_by_addr(addr)
                        if client == None:
                            continue
                        if port not in list(client['devices']):
                            continue
                        if terminal['socket'] in client['devices'][port]['log_subscribe']:
                            continue
                        client['devices'][port]['log_subscribe'].append(terminal['socket'])
                        print "terminal {0}:{1}".format(terminal['addr'][0], terminal['addr'][1]),
                        print "subscribed log of device {0}:{1}".format(values[0], port)
                    elif type == TBframe.LOG_UNSUB:
                        values = value.split(',')
                        if len(values) != 3:
                            continue
                        addr = (values[0], int(values[1]))
                        port = values[2]
                        client = self.get_client_by_addr(addr)
                        if client == None:
                            continue
                        if port not in list(client['devices']):
                            continue
                        if terminal['socket'] not in client['devices'][port]['log_subscribe']:
                            continue
                        client['devices'][port]['log_subscribe'].remove(terminal['socket'])
                        print "terminal {0}:{1}".format(terminal['addr'][0], terminal['addr'][1]),
                        print "unsubscribed log of device {0}:{1}".format(values[0], port)
                    elif type == TBframe.STATUS_SUB:
                        values = value.split(',')
                        if len(values) != 3:
                            continue
                        addr = (values[0], int(values[1]))
                        port = values[2]
                        client = self.get_client_by_addr(addr)
                        if client == None:
                            continue
                        if port not in list(client['devices']):
                            continue
                        if terminal['socket'] in client['devices'][port]['status_subscribe']:
                            continue
                        client['devices'][port]['status_subscribe'].append(terminal['socket'])
                        print "terminal {0}:{1}".format(terminal['addr'][0], terminal['addr'][1]),
                        print "subscribed status of device {0}:{1}".format(values[0], port)
                        content = client['addr'][0] + ',' + str(client['addr'][1]) + ',' + port
                        content += ':' + client['devices'][port]['status']
                        data = TBframe.construct(TBframe.DEVICE_STATUS, content)
                        terminal['socket'].send(data)
                    elif type == TBframe.STATUS_UNSUB:
                        values = value.split(',')
                        if len(values) != 3:
                            continue
                        addr = (values[0], int(values[1]))
                        port = values[2]
                        client = self.get_client_by_addr(addr)
                        if client == None:
                            continue
                        if port not in list(client['devices']):
                            continue
                        if terminal['socket'] not in client['devices'][port]['status_subscribe']:
                            continue
                        client['devices'][port]['status_subscribe'].remove(terminal['socket'])
                        print "terminal {0}:{1}".format(terminal['addr'][0], terminal['addr'][1]),
                        print "unsubscribed status of device {0}:{1}".format(values[0], port)
                    elif type == TBframe.LOG_DOWNLOAD:
                        values = value.split(',')
                        if len(values) != 3:
                            continue
                        addr = (values[0], int(values[1]))
                        port = values[2]
                        filename = 'server/' + addr[0] + '-' + port[5:] + '.log'
                        client = self.get_client_by_addr(addr)
                        if client == None or port not in list(client['devices']) or os.path.exists(filename) == False:
                            data = TBframe.construct(TBframe.CMD_ERROR,'fail')
                            terminal['socket'].send(data)
                            print "terminal {0}:{1}".format(terminal['addr'][0], terminal['addr'][1]),
                            print "downloading log of device {0}:{1} ...".format(values[0], port),
                            print "failed"
                            continue
                        self.send_file_to_someone(terminal, filename)
                        heartbeat_timeout = time.time() + 30
                        data = TBframe.construct(TBframe.CMD_DONE, 'success')
                        terminal['socket'].send(data)
                        print "terminal {0}:{1}".format(terminal['addr'][0], terminal['addr'][1]),
                        print "downloading log of device {0}:{1} ...".format(values[0], port),
                        print "succeed"
            except socket.timeout:
                continue
            except:
                if DEBUG:
                    raise
                break
        for client in self.client_list:
            for port in list(client['devices']):
                if terminal['socket'] in client['devices'][port]['log_subscribe']:
                    client['devices'][port]['log_subscribe'].remove(terminal['socket'])
                if terminal['socket'] in client['devices'][port]['status_subscribe']:
                    client['devices'][port]['status_subscribe'].remove(terminal['socket'])
        for device in using_list:
            addr = device[0]
            port = device[1]
            client = self.get_client_by_addr(addr)
            if client != None and port in list(client['devices']):
                with client['devices'][port]['lock']:
                    if client['devices'][port]['using'] > 0:
                        client['devices'][port]['using'] -= 1
        terminal['socket'].close()
        print "terminal ", terminal['addr'], "disconnected"
        self.terminal_list.remove(terminal)
        self.send_device_list_to_all()

    def client_listen_thread(self):
        self.client_socket.listen(5)
        while self.keep_running:
            conn, addr = self.client_socket.accept()
            client = {'socket':conn, 'addr':addr, 'devices':{}}
            print "client ", addr," connected"
            self.client_list.append(client)
            thread.start_new_thread(self.client_serve_thread, (client,))

    def terminal_listen_thread(self):
        self.terminal_socket.listen(5)
        while self.keep_running:
            conn, addr = self.terminal_socket.accept()
            terminal = {'socket':conn, 'addr':addr}
            print "terminal ", addr," connected"
            self.terminal_list.append(terminal)
            thread.start_new_thread(self.terminal_serve_thread, (terminal,))

    def init(self, server_port):
        try:
            #initilize CLIENT socket
            self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.client_socket.bind(('', server_port))
            #initilize TERMINAL socket
            self.terminal_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.terminal_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.terminal_socket.bind(('', server_port + 1))
        except:
            print "address still in use, try later"
            return "fail"
        if os.path.exists('server') == False:
            os.mkdir('server')
        return "success"

    def run(self):
        try:
            client_thread = thread.start_new_thread(self.client_listen_thread, ())
            terminal_thread = thread.start_new_thread(self.terminal_listen_thread, ())
            while True:
                time.sleep(0.1)
                if self.allocated['devices'] != [] and time.time() > self.allocated['timeout']:
                    with self.allocated['lock']:
                        self.allocated['devices'] = []
        except:
            self.keep_running = False

    def deinit(self):
        for c in self.client_list:
            c['socket'].close()
        for t in self.terminal_list:
            t['socket'].close()
        try:
            self.client_socket.close()
            self.terminal_socket.close()
        except:
            pass

    def server_func(self, server_port):
        if self.init(server_port) == "success":
            self.run()
        self.deinit()
