import os, sys, time, socket, ssl, signal, re
import thread, threading, json, traceback, shutil
import TBframe

MAX_MSG_LENTH = 8192
ENCRYPT_CLIENT = False
ENCRYPT_TERMINAL = False
ENCRYPT_CONTROLLER = False
DEBUG = True

def signal_handler(sig, frame):
    print "received SIGINT"
    raise KeyboardInterrupt

class Server:
    def __init__(self):
        self.keyfile = 'server_key.pem'
        self.certfile = 'server_cert.pem'
        self.client_socket = None
        self.terminal_socket = None
        self.controller_socket = None
        self.clients = {}
        self.terminals = {}
        self.conn_timeout = {}
        self.device_subscribe_map = {}
        self.keep_running = True
        self.log_preserve_period = (7 * 24 * 3600) * 3 #save log for 3 weeks
        self.allocated = {'lock':threading.Lock(), 'devices':[], 'timeout':0}
        self.special_purpose_set = {'mk3060-alink':[], 'esp32-alink':[]}
        #mk3060
        self.special_purpose_set['mk3060-alink'] += ['DN02QRKQ', 'DN02RDVL', 'DN02RDVT', 'DN02RDVV']
        self.special_purpose_set['mk3060-alink'] += ['DN02X2ZO', 'DN02X2ZS', 'DN02X2ZX', 'DN02X2ZZ']
        self.special_purpose_set['mk3060-alink'] += ['DN02X303', 'DN02X304', 'DN02X30B', 'DN02X30H']
        self.special_purpose_set['mk3060-mesh'] = self.special_purpose_set['mk3060-alink']
        #esp32
        self.special_purpose_set['esp32-alink'] += ['espif-3.1.1', 'espif-3.1.2', 'espif-3.1.3', 'espif-3.1.4']
        self.special_purpose_set['esp32-alink'] += ['espif-3.2.1', 'espif-3.2.2', 'espif-3.2.3', 'espif-3.2.4']
        self.special_purpose_set['esp32-alink'] += ['espif-3.3.1', 'espif-3.3.2', 'espif-3.3.3', 'espif-3.3.4']
        self.special_purpose_set['esp32-mesh'] = self.special_purpose_set['esp32-alink']

    def send_device_list_to_terminal(self, uuid):
        if uuid not in self.terminals or self.terminals[uuid]['valid'] == False:
            return
        devices = []
        for device in self.terminals[uuid]['devices']:
            [cuuid, port] = device.split(':')
            if cuuid in self.clients and port in self.clients[cuuid]['devices'] and \
                    self.clients[cuuid]['devices'][port]['valid'] == True:
                devices.append(cuuid + ',' + port + '|' + str(self.clients[cuuid]['devices'][port]['using']))
            else:
                devices.append(cuuid + ',' + port + '|' + '-1')
        dev_str = ':'.join(devices)
        data = TBframe.construct(TBframe.ALL_DEV, dev_str)
        self.terminals[uuid]['socket'].send(data)

    def client_serve_thread(self, conn, addr):
        file = {}
        msg = ''
        client = None
        self.conn_timeout[conn] = {'type':'client', 'addr': addr, 'timeout': time.time() + 30}
        while self.keep_running:
            try:
                new_msg = conn.recv(MAX_MSG_LENTH)
                if new_msg == '':
                    break

                msg += new_msg
                while msg != '':
                    type, length, value, msg = TBframe.parse(msg)
                    if type == TBframe.TYPE_NONE:
                        break

                    if client == None:
                        if type != TBframe.CLIENT_LOGIN:
                            data = TBframe.construct(TBframe.CLIENT_LOGIN, 'request')
                            conn.send(data)
                            time.sleep(0.1)
                            continue
                        try:
                            [uuid, tag, token] = value.split(',')
                        except:
                            if DEBUG: traceback.print_exc()
                        if uuid not in self.clients or self.clients[uuid]['token'] != token:
                            print "warning: invalid client {0} connecting @ {1}".format(value, addr)
                            data = TBframe.construct(TBframe.CLIENT_LOGIN, 'fail')
                            conn.send(data)
                            self.conn_timeout[conn]['timeout'] = time.time() + 1
                            break
                        else:
                            client = self.clients[uuid]
                            client['socket'] = conn
                            client['tag'] = tag
                            client['addr'] = addr
                            client['valid'] = True
                            data = TBframe.construct(TBframe.CLIENT_LOGIN, 'success')
                            conn.send(data)
                            self.conn_timeout[conn]['timeout'] = time.time() + 30
                            self.report_status_to_controller()
                            print "client {0} connected @ {1}, tag={2}".format(uuid, addr, repr(tag))
                        continue

                    self.conn_timeout[conn]['timeout'] = time.time() + 30
                    if type == TBframe.CLIENT_DEV:
                        new_devices = value.split(':')
                        device_list_changed = False
                        for port in new_devices:
                            if port == "":
                                continue
                            if port in client['devices'] and client['devices'][port]['valid'] == True:
                                continue
                            device_list_changed = True
                            if port not in client['devices']:
                                print "new device {0} added to client {1}".format(port, client['uuid'])
                                client['devices'][port] = {
                                        'lock':threading.Lock(),
                                        'valid':True,
                                        'using':0,
                                        'status':'{}'}
                                dev_str = client['uuid'] + ':' + port
                                if dev_str in self.device_subscribe_map:
                                    self.send_device_list_to_terminal(self.device_subscribe_map[dev_str])
                            else:
                                print "device {0} re-added to client {1}".format(port, client['uuid'])
                                client['devices'][port]['status'] = '{}'
                                client['devices'][port]['valid'] = True
                                dev_str = client['uuid'] + ':' + port
                                if dev_str in self.device_subscribe_map:
                                    self.send_device_list_to_terminal(self.device_subscribe_map[dev_str])

                        for port in list(client['devices']):
                            if port in new_devices:
                                continue
                            if client['devices'][port]['valid'] == False:
                                continue
                            device_list_changed = True
                            client['devices'][port]['status'] = '{}'
                            client['devices'][port]['valid'] = False
                            print "device {0} removed from client {1}".format(port, client['uuid'])
                            dev_str = client['uuid'] + ':' + port
                            if dev_str in self.device_subscribe_map:
                                self.send_device_list_to_terminal(self.device_subscribe_map[dev_str])

                        if device_list_changed:
                            self.report_status_to_controller()

                        for port in list(file):
                            if client['devices'][port]['valid'] == True:
                                continue
                            file[port]['handle'].close()
                            file.pop(port)
                    elif type == TBframe.DEVICE_LOG:
                        port = value.split(':')[0]
                        if port not in client['devices']:
                            continue
                        #forwad log to subscribed devices
                        dev_str = client['uuid'] + ':' + port
                        if dev_str in self.device_subscribe_map and client['tag'] not in value:
                            log = client['uuid'] + ',' + value
                            data = TBframe.construct(type, log)
                            uuid = self.device_subscribe_map[dev_str]
                            try:
                                self.terminals[uuid]['socket'].send(data)
                            except:
                                continue

                        #save log to files
                        try:
                            logtime = value.split(':')[1]
                            logstr = value[len(port) + 1 + len(logtime):]
                            logtime = float(logtime)
                            logtimestr = time.strftime("%Y-%m-%d@%H:%M:%S", time.localtime(logtime))
                            logtimestr += ("{0:.3f}".format(logtime-int(logtime)))[1:]
                            logstr = logtimestr + logstr
                            logdatestr = time.strftime("%Y-%m-%d", time.localtime(logtime))
                        except:
                            if DEBUG: traceback.print_exc()
                            continue
                        if (port not in file) or (file[port]['date'] != logdatestr):
                            if port in file:
                                file[port]['handle'].close()
                                file.pop(port)
                            log_dir = 'server/' + logdatestr
                            if os.path.isdir(log_dir) == False:
                                try:
                                    os.mkdir(log_dir)
                                except:
                                    print "error: can not create directory {0}".format(log_dir)
                            filename = log_dir + '/' + client['uuid'] + '-' + port.split('/')[-1]  + '.log'
                            try:
                                handle = open(filename, 'a+')
                                file[port] = {'handle':handle, 'date': logdatestr}
                            except:
                                print "error: can not open/create file ", filename
                        if port in file and file[port]['date'] == logdatestr:
                            file[port]['handle'].write(logstr)
                    elif type == TBframe.DEVICE_STATUS:
                        #print value
                        port = value.split(':')[0]
                        if port not in client['devices']:
                            continue
                        client['devices'][port]['status'] = value[len(port)+1:]
                        dev_str = client['uuid'] + ':' + port
                        if dev_str in self.device_subscribe_map:
                            log = client['uuid'] + ',' + port
                            log += value[len(port):]
                            data = TBframe.construct(type, log)
                            uuid = self.device_subscribe_map[dev_str]
                            try:
                                self.terminals[uuid]['socket'].send(data)
                            except:
                                continue
                    elif type == TBframe.DEVICE_ERASE or type == TBframe.DEVICE_PROGRAM or \
                         type == TBframe.DEVICE_START or type == TBframe.DEVICE_STOP or \
                         type == TBframe.DEVICE_RESET or type == TBframe.DEVICE_CMD or \
                         type == TBframe.FILE_BEGIN or type == TBframe.FILE_DATA or \
                         type == TBframe.FILE_END:
                        values = value.split(',')
                        uuid = values[0]
                        if uuid in self.terminals and self.terminals[uuid]['valid'] == True:
                            if values[1] != 'success' and values[1] != 'ok':
                                data = TBframe.construct(TBframe.CMD_ERROR, ','.join(values[1:]))
                            else:
                                data = TBframe.construct(TBframe.CMD_DONE, ','.join(values[1:]))
                            self.terminals[uuid]['socket'].send(data)
            except:
                if DEBUG: traceback.print_exc()
                break
        conn.close()
        if conn in self.conn_timeout: self.conn_timeout.pop(conn)
        if client:
            for port in client['devices']:
                if client['devices'][port]['valid'] == False:
                    continue
                client['devices'][port]['status'] = '{}'
                client['devices'][port]['valid'] = False
                print "device {0} removed from client {1}".format(port, client['uuid'])
                dev_str = client['uuid'] + ':' + port
                if dev_str not in self.device_subscribe_map:
                    continue
                uuid = self.device_subscribe_map[dev_str]
                self.send_device_list_to_terminal(uuid)
            client['valid'] = False
            print "client {0} @ {1} disconnected".format(client['uuid'], addr)
            self.report_status_to_controller()
        else:
            print "client @ {0} disconnected".format(addr)

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

    def allocate_devices(self, value):
        values = value.split(',')
        if len(values) < 2:
            return ['error','argument']

        model = values[0]
        model = model.lower()

        number = values[1]
        try:
            number = int(number)
        except:
            return ['error','argument']
        if number <= 0:
            return ['error','argument']

        purpose = 'general'
        if len(values) > 2:
            purpose = values[2]
        func_set = None
        if purpose != 'general':
            func_set = model + '-' + purpose
            if func_set not in self.special_purpose_set:
                print "error: allocate {0} for {1} purpose not supported".format(model, purpose)
                return ['error','argument']
            func_set = self.special_purpose_set[func_set]
        if DEBUG and func_set: print purpose, func_set

        allocated = []
        with self.allocated['lock']:
            for client in self.client_list:
                allocated = []
                ports = list(client['devices'])
                ports.sort()
                for port in ports:
                    if client['devices'][port]['valid'] == False: #no exist
                        continue

                    if client['devices'][port]['using'] != 0: #busy
                        continue

                    if port in self.allocated['devices']: #in allocated buffer
                        continue

                    try:
                        status = json.loads(client['devices'][port]['status'])
                    except:
                        print 'parse {0} status failed'.format(port)
                        status = None
                    if status:
                        if 'model' not in status or model != status['model'].lower():
                            continue
                    else:
                        paths = {'mk3060':'mxchip', 'esp32':'espif'}
                        if model not in paths:
                            continue
                        pathstr = paths[model]
                        if pathstr not in port:
                            continue

                    if func_set:
                        match = False
                        for devicestr in func_set:
                            if devicestr not in port:
                                continue
                            match = True
                            break
                        if match == False:
                            continue

                    allocated.append(port)
                    if len(allocated) >= number:
                        break
                if len(allocated) >= number:
                    break
            if len(allocated) >= number:
                self.allocated['devices'] += allocated
                self.allocated['timeout'] = time.time() + 10
                if DEBUG: print "allocated", allocated
                return ['success', '|'.join(allocated)]
            else:
                if DEBUG: print "allocate failed"
                return ['fail', 'busy']

    def increase_device_refer(self, client, port, using_list):
        if [client['uuid'], port] in using_list:
            return
        if port not in list(client['devices']):
            return
        with client['devices'][port]['lock']:
            client['devices'][port]['using'] += 1
        using_list.append([client['uuid'], port])
        dev_str = client['uuid'] + ':' + port
        if dev_str not in self.device_subscribe_map:
            return
        uuid = self.device_subscribe_map[dev_str]
        self.send_device_list_to_terminal(uuid)


    def terminal_serve_thread(self, conn, addr):
        using_list = []
        msg = ''
        terminal = None
        self.conn_timeout[conn] = {'type': 'terminal', 'addr': addr, 'timeout': time.time() + 30}
        while self.keep_running:
            try:
                new_msg = conn.recv(MAX_MSG_LENTH);
                if new_msg == '':
                    break

                msg += new_msg
                while msg != '':
                    type, length, value, msg = TBframe.parse(msg)
                    if type == TBframe.TYPE_NONE:
                        break

                    self.conn_timeout[conn]['timeout'] = time.time() + 30
                    if terminal == None:
                        if type != TBframe.TERMINAL_LOGIN:
                            data = TBframe.construct(TBframe.CLIENT_LOGIN, 'request')
                            conn.send(data)
                            time.sleep(0.1)
                            continue
                        try:
                            [uuid, token] = value.split(',')
                        except:
                            if DEBUG: traceback.print_exc()
                        if uuid not in self.terminals or self.terminals[uuid]['token'] != token:
                            print "warning: invalid terminal {0} connecting @ {1}".format(value, addr)
                            data = TBframe.construct(TBframe.TERMINAL_LOGIN, 'fail')
                            conn.send(data)
                            self.conn_timeout[conn]['timeout'] = time.time() + 1
                            break
                        else:
                            terminal = self.terminals[uuid]
                            terminal['socket'] = conn
                            terminal['addr'] = addr
                            terminal['valid'] = True
                            data = TBframe.construct(TBframe.TERMINAL_LOGIN, 'success')
                            conn.send(data)
                            self.conn_timeout[conn]['timeout'] = time.time() + 30
                            print "terminal {0} connected @ {1}".format(uuid, addr)
                            self.send_device_list_to_terminal(terminal['uuid'])
                            self.report_status_to_controller()
                        continue

                    self.conn_timeout[conn]['timeout'] = time.time() + 30
                    if type == TBframe.FILE_BEGIN or type == TBframe.FILE_DATA or type == TBframe.FILE_END:
                        dev_str = value.split(':')[0]
                        uuid = dev_str.split(',')[0]
                        if uuid not in self.clients or self.clients[uuid]['valid'] == False:
                            data = TBframe.construct(TBframe.CMD_ERROR, 'nonexist')
                            conn.send(data)
                            continue
                        client = self.clients[uuid]
                        content = terminal['uuid'] + value[len(dev_str):]
                        data = TBframe.construct(type, content)
                        client['socket'].send(data)
                    elif type == TBframe.DEVICE_ERASE or type == TBframe.DEVICE_PROGRAM or \
                         type == TBframe.DEVICE_START or type == TBframe.DEVICE_STOP or \
                         type == TBframe.DEVICE_RESET or type == TBframe.DEVICE_CMD:
                        dev_str_split = value.split(':')[0].split(',')[0:2]
                        if len(dev_str_split) != 2:
                            data = TBframe.construct(TBframe.CMD_ERROR,'argerror')
                            conn.send(data)
                            continue
                        [uuid, port] = dev_str_split
                        if uuid not in self.clients or self.clients[uuid]['valid'] == False:
                            data = TBframe.construct(TBframe.CMD_ERROR,'nonexist')
                            conn.send(data)
                            continue
                        client = self.clients[uuid]
                        content = terminal['uuid'] + value[len(uuid):]
                        data = TBframe.construct(type, content)
                        client['socket'].send(data)
                        self.increase_device_refer(client, port, using_list)
                    elif type == TBframe.DEVICE_ALLOC:
                        result = self.allocate_devices(value)
                        content = ','.join(result)
                        data = TBframe.construct(TBframe.DEVICE_ALLOC, content)
                        conn.send(data)
                    elif type == TBframe.LOG_DOWNLOAD:
                        dev_str_split = value.split(',')
                        if len(dev_str_split) != 2:
                            continue
                        [uuid, port] = dev_str_split
                        datestr = time.strftime('%Y-%m-%d')
                        filename = 'server/' + datestr + '/' + uuid + '-' + port.split('/')[-1] + '.log'
                        if uuid not in self.clients or self.clients[uuid]['valid'] == False or \
                           port not in self.clients[uuid]['devices'] or os.path.exists(filename) == False:
                            data = TBframe.construct(TBframe.CMD_ERROR,'fail')
                            conn.send(data)
                            print "terminal {0}:{1}".format(terminal['addr'][0], terminal['addr'][1]),
                            print "downloading log of device {0}:{1} ... failed".format(uuid, port)
                            continue
                        self.send_file_to_someone(terminal, filename)
                        heartbeat_timeout = time.time() + 30
                        data = TBframe.construct(TBframe.CMD_DONE, 'success')
                        conn.send(data)
                        print "terminal {0}:{1}".format(terminal['addr'][0], terminal['addr'][1]),
                        print "downloading log of device {0}:{1} ... succeed".format(uuid, port)
            except:
                if DEBUG: traceback.print_exc()
                break
        for device in using_list:
            uuid = device[0]
            port = device[1]
            client = None
            for c in self.clients:
                c = self.clients[c]
                if c['uuid'] != uuid:
                    continue
                client = c
                break
            if client != None and port in list(client['devices']):
                with client['devices'][port]['lock']:
                    if client['devices'][port]['using'] > 0:
                        client['devices'][port]['using'] -= 1
        conn.close()
        if conn in self.conn_timeout: self.conn_timeout.pop(conn)
        print "terminal", addr, "disconnected"
        if terminal:
            terminal['valid'] = False
            self.report_status_to_controller()

    def client_listen_thread(self):
        self.client_socket.listen(5)
        if ENCRYPT_CLIENT:
            self.client_socket = ssl.wrap_socket(self.client_socket, self.keyfile, self.certfile, True)
        while self.keep_running:
            try:
                (conn, addr) = self.client_socket.accept()
                thread.start_new_thread(self.client_serve_thread, (conn, addr,))
            except:
                traceback.print_exc()

    def terminal_listen_thread(self):
        self.terminal_socket.listen(5)
        if ENCRYPT_TERMINAL:
            self.terminal_socket = ssl.wrap_socket(self.terminal_socket, self.keyfile, self.certfile, True)
        while self.keep_running:
            try:
                (conn, addr) = self.terminal_socket.accept()
                thread.start_new_thread(self.terminal_serve_thread, (conn, addr,))
                print "terminal ", addr," connected"
            except:
                traceback.print_exc()

    def report_status_to_controller(self):
        if self.controller_socket == None:
            return
        clients = []; devices = []; terminals = []
        for uuid in self.clients:
            if self.clients[uuid]['valid'] == False:
                continue
            clients += [uuid]
            device_list = self.clients[uuid]['devices']
            for port in device_list:
                if device_list[port]['valid'] == False:
                    continue
                dev_str = uuid + ':' + port
                devices += [dev_str]
        for uuid in self.terminals:
            if self.terminals[uuid]['valid'] == False:
                continue
            terminals += [uuid]
        status = {'clients':clients, 'devices':devices, 'terminals': terminals}
        content = TBframe.construct(TBframe.ACCESS_REPORT_STATUS, json.dumps(status))
        try:
            self.controller_socket.send(content)
        except:
            pass

    def controller_interact_thread(self, controller_ip, controller_port):
        sock = None; logedin = False; status_timeout = False
        while self.keep_running:
            if sock == None: #connect to controller
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                if ENCRYPT_CONTROLLER:
                    sock = ssl.wrap_socket(sock, cert_reqs=ssl.CERT_REQUIRED, ca_certs='server_cert.pem')
                try:
                    sock.connect((controller_ip, controller_port))
                    msg = ''; logedin = False; status_timeout = None
                    sock.settimeout(1)
                except:
                    if DEBUG: traceback.print_exc()
                    sock = None; logedin = False; status_timeout = None
                    self.controller_socket = sock
                    time.sleep(2)
                    continue

            while sock != None:
                if logedin == False: #try to login
                    content = {'client_port':self.client_socket.getsockname()[1], 'terminal_port':self.terminal_socket.getsockname()[1]}
                    content = 'server,' + json.dumps(content)
                    content = TBframe.construct(TBframe.ACCESS_LOGIN, content)
                    try:
                        sock.send(content)
                    except:
                        sock = None; logedin = False; status_timeout = None
                        self.controller_socket = sock
                        continue

                try:
                    data = sock.recv(MAX_MSG_LENTH)
                except socket.timeout:
                    continue
                except:
                    sock = None; logedin = False; status_timeout = None
                    self.controller_socket = sock
                    break
                if not data:
                    print("error: connection to controller lost")
                    sock = None; logedin = False; status_timeout = None
                    self.controller_socket = sock
                    break

                msg += data
                while msg != '':
                    type, length, value, msg = TBframe.parse(msg)
                    #print time.time(), 'controller', type, value
                    if type == TBframe.TYPE_NONE:
                        break

                    if type == TBframe.ACCESS_LOGIN:
                        if value == 'ok':
                            logedin = True
                            status_timeout = time.time() + 10
                            self.controller_socket = sock
                            self.report_status_to_controller()
                        else:
                            time.sleep(5)
                        continue
                    if type == TBframe.ACCESS_REPORT_STATUS:
                        continue
                    if type == TBframe.ACCESS_ADD_CLIENT:
                        try:
                            [uuid, token] = value.split(',')
                        except:
                            continue
                        if uuid not in self.clients:
                            client = {'uuid': uuid,
                                      'token': token,
                                      'valid':False,
                                      'devices':{}}
                            self.clients[uuid] = client
                        else:
                            client = self.clients[uuid]
                            if 'sock' in client:
                                client['sock'].close()
                            client['valid'] = False
                            client['token'] = token
                        content = TBframe.construct(TBframe.ACCESS_ADD_CLIENT, 'success,' + value)
                        try:
                            sock.send(content)
                        except:
                            sock = None; logedin = False; status_timeout = None
                            self.controller_socket = sock
                        continue
                    if type == TBframe.ACCESS_DEL_CLIENT:
                        uuid = value
                        if uuid not in self.clients:
                            continue
                        client = self.clients[uuid]
                        if 'socket' in client:
                            client['socket'].close()
                        client['token'] = None
                        client['valid'] = False
                        continue
                    if type == TBframe.ACCESS_ADD_TERMINAL:
                        try:
                            [uuid, token, devices] = value.split(',')
                            devices = devices.split('|')
                        except:
                            traceback.print_exc()
                            continue
                        if uuid not in self.terminals:
                            terminal = {'uuid': uuid,
                                        'token': token,
                                        'valid':False,
                                        'devices':devices}
                            self.terminals[uuid] = terminal
                        else:
                            terminal = self.terminals[uuid]
                            if 'socket' in terminal:
                                terminal['socket'].close()
                            for device in terminal['devices']:
                                if device not in self.device_subscribe_map:
                                    continue
                                self.device_subscribe_map.pop(device)
                            terminal['valid'] = False
                            terminal['token'] = token
                            terminal['devices'] = devices
                        for device in devices:
                            self.device_subscribe_map[device] = uuid
                        content = ','.join(['success', uuid, token])
                        content = TBframe.construct(TBframe.ACCESS_ADD_TERMINAL, content)
                        try:
                            sock.send(content)
                        except:
                            sock = None; logedin = False; status_timeout = None
                            self.controller_socket = sock
                        continue
                    if type == TBframe.ACCESS_DEL_TERMINAL:
                        continue

    def house_keeping_thread(self):
        minute = time.strftime("%Y-%m-%d@%H:%M")
        datestr = time.strftime("%Y-%m-%d")
        statistics={ \
                'terminal_num_max':0, \
                'client_num_max':0, \
                'device_num_max':0, \
                'device_use_max':0, \
                'terminal_num_avg':0, \
                'client_num_avg':0, \
                'device_num_avg':0, \
                'device_use_avg':0 \
                }
        statistics_cnt = 0
        logname='statistics.log'
        try:
            f = open(logname, 'a+')
        except:
            print "error: unable to create/open {0}".format(logname)
            return
        while self.keep_running:
            time.sleep(3)

            #remove outdated log files
            if time.strftime("%Y-%m-%d") != datestr:
                tbefore = time.mktime(time.strptime(time.strftime('%Y-%m-%d'), '%Y-%m-%d'))
                tbefore -= self.log_preserve_period
                flist = os.listdir('server')
                for fname in flist:
                    if os.path.isdir('server/'+ fname) == False:
                        continue
                    if re.match('[0-9]{4}-[0-9]{2}-[0-9]{2}', fname) == None:
                        continue
                    ftime = time.strptime(fname, '%Y-%m-%d')
                    ftime = time.mktime(ftime)
                    if ftime >= tbefore:
                        continue
                    shutil.rmtree('server/' + fname)
                datestr = time.strftime("%Y-%m-%d")

            #disconnect timeout connections
            now = time.time()
            for conn in list(self.conn_timeout):
                if now <= self.conn_timeout[conn]['timeout']:
                    continue
                conn.close()
                print self.conn_timeout[conn]['type'], self.conn_timeout[conn]['addr'], 'timeout'
                self.conn_timeout.pop(conn)

            #generate and save statistics data
            client_cnt = 0; terminal_cnt = 0
            device_cnt = 0; device_use = 0

            for uuid in self.clients:
                client = self.clients[uuid]
                if client['valid'] == False:
                    continue
                client_cnt += 1
                devices = client['devices']
                for port in devices:
                    if devices[port]['valid'] == False:
                        continue
                    device_cnt += 1
                    if devices[port]['using'] <= 0:
                        continue
                    device_use += 1
            for uuid in self.terminals:
                if self.terminals[uuid]['valid'] == False:
                    continue
                terminal_cnt += 1

            if terminal_cnt > statistics['terminal_num_max']:
                statistics['terminal_num_max'] = terminal_cnt
            if client_cnt > statistics['client_num_max']:
                statistics['client_num_max'] = client_cnt
            if device_cnt > statistics['device_num_max']:
                statistics['device_num_max'] = device_cnt
            if device_use > statistics['device_use_max']:
                statistics['device_use_max'] = device_use
            statistics['terminal_num_avg'] += terminal_cnt
            statistics['client_num_avg'] += client_cnt
            statistics['device_num_avg'] += device_cnt
            statistics['device_use_avg'] += device_use
            statistics_cnt += 1.0

            now = time.strftime("%Y-%m-%d@%H:%M")
            if now == minute:
                continue
            statistics['terminal_num_avg'] = round(statistics['terminal_num_avg']/statistics_cnt, 2)
            statistics['client_num_avg'] = round(statistics['client_num_avg']/statistics_cnt, 2)
            statistics['device_num_avg'] = round(statistics['device_num_avg']/statistics_cnt, 2)
            statistics['device_use_avg'] = round(statistics['device_use_avg']/statistics_cnt, 2)
            data = json.dumps({minute:statistics}, sort_keys=True) + '\n'
            f.write(data)
            f.flush()
            minute = now
            statistics={ \
                    'terminal_num_max':0, \
                    'client_num_max':0, \
                    'device_num_max':0, \
                    'device_use_max':0, \
                    'terminal_num_avg':0, \
                    'client_num_avg':0, \
                    'device_num_avg':0, \
                    'device_use_avg':0 \
                    }
            statistics_cnt = 0
            if os.path.isfile(logname) == True:
                continue
            try:
                f.close()
                f = open(logname, 'a+')
            except:
                print "error: unable to create/open {0}".format(logname)
                return

    def init(self):
        try:
            #initilize CLIENT socket
            client_port = 2048 + ord(os.urandom(1))
            self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.client_socket.bind(('', client_port))
            #initilize TERMINAL socket
            terminal_port = 2048 + ord(os.urandom(1))
            while terminal_port == client_port:
                terminal_port = 2048 + ord(os.urandom(1))
            self.terminal_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.terminal_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.terminal_socket.bind(('', terminal_port))
        except:
            print "address still in use, try later"
            return "fail"
        if os.path.exists('server') == False:
            os.mkdir('server')
        return "success"

    def run(self, host_name, host_port):
        signal.signal(signal.SIGINT, signal_handler)
        try:
            thread.start_new_thread(self.client_listen_thread, ())
            thread.start_new_thread(self.terminal_listen_thread, ())
            thread.start_new_thread(self.controller_interact_thread, (host_name, host_port,))
            thread.start_new_thread(self.house_keeping_thread, ())
            while True:
                time.sleep(0.1)
                if self.allocated['devices'] != [] and time.time() > self.allocated['timeout']:
                    with self.allocated['lock']:
                        self.allocated['devices'] = []
        except:
            print "server exiting ..."
            self.keep_running = False

    def deinit(self):
        sockets = []
        for uuid in self.clients:
            if self.clients[uuid]['valid'] == False:
                continue
            if 'socket' not in self.clients[uuid]:
                continue
            sockets.append(self.clients[uuid]['socket'])
        for uuid in self.terminals:
            if self.terminals[uuid]['valid'] == False:
                continue
            if 'socket' not in self.terminals[uuid]:
                continue
            sockets.append(self.terminals[uuid]['socket'])
        for sock in [self.client_socket, self.terminal_socket, self.controller_socket]:
            if not sock:
                continue
            sockets.append(sock)
        for sock in sockets:
            try:
                sock.close()
            except:
                pass

    def server_func(self, host_name, host_port):
        if self.init() == "success":
            self.run(host_name, host_port)
        self.deinit()
