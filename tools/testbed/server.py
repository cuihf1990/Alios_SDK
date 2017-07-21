import os, sys, time, socket
import subprocess, thread
import TBframe

MAX_MSG_LENTH      = 2000

class Server:
    def __init__(self):
        self.client_socket = 0
        self.terminal_socket = 0
        self.client_list = []
        self.terminal_list = []
        self.keep_running = True

    def construct_dev_list(self):
        l = []
        for c in self.client_list:
            l.append(','.join([c['addr'][0], str(c['addr'][1])] + list(c['devices'])))
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
        while self.keep_running:
            try:
                if time.time() > heartbeat_timeout:
                    print "client {0} heartbeat timeout".format(client['addr'])
                    break

                msg = client['socket'].recv(MAX_MSG_LENTH)
                if msg == '':
                    break
                while msg != '':
                    type, length, value, msg = TBframe.parse(msg)
                    if type == TBframe.TYPE_NONE:
                        break

                    if type == TBframe.CLIENT_DEV:
                        new_devices = value.split(':')
                        for port in new_devices:
                            if port != "" and port not in client['devices']:
                                print "device {0} added to client {1}".format(port, client['addr'])
                                client['devices'][port] = []
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
                        file[port].write(value[len(port) + 1:])
                        if client['devices'][port] != []:
                            log = client['addr'][0] + ',' + str(client['addr'][1]) + ',' + port
                            log += value[len(port):]
                            data = TBframe.construct(TBframe.DEVICE_LOG, log)
                            for s in client['devices'][port]:
                                try:
                                    s.send(data)
                                except:
                                    continue
                    elif type == TBframe.DEVICE_ERASE or type == TBframe.DEVICE_PROGRAM:
                        values = value.split(',')
                        addr = (values[0], int(values[1]))
                        terminal = ''
                        for t in self.terminal_list:
                            if t['addr'] == addr:
                                terminal = t
                        if terminal != '':
                            if values[2] != 'success':
                                data = TBframe.construct(TBframe.CMD_ERROR, '')
                            else:
                                data = TBframe.construct(TBframe.CMD_DONE, '')
                            terminal['socket'].send(data)
                    elif type == TBframe.HEARTBEAT:
                        heartbeat_timeout = time.time() + 30
            except socket.timeout:
                continue
            except:
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

    def terminal_serve_thread(self, terminal):
        self.send_device_list_to_terminal(terminal)
        terminal['socket'].settimeout(1)
        heartbeat_timeout = time.time() + 30
        msg = ''
        filename = ''
        filetransmitting = False
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

                    if type == TBframe.FILE_BEGIN:
                        if 'file' in locals() and file.closed == False:
                            file.close()
                        filename = 'server/' + value + "-" + terminal['addr'][0]
                        filename += "@" + time.strftime("%Y-%m-%d-%H-%M")
                        file = open(filename, 'w')
                    elif type == TBframe.FILE_DATA:
                        if 'file' in locals() and file.closed == False:
                            file.write(value)
                    elif type == TBframe.FILE_END:
                        if 'file' in locals():
                            file.close()
                        print "file {0} reveived from {1}".format(filename.split('/')[-1], terminal['addr'])
                    elif type == TBframe.FILE_COPY:
                        dst = value.split(',')
                        addr = (dst[0], int(dst[1]))
                        client = self.get_client_by_addr(addr)
                        if client == None:
                            data = TBframe.construct(TBframe.CMD_ERROR,'')
                            terminal['socket'].send(data)
                            continue
                        self.send_file_to_someone(client, filename)
                        data = TBframe.construct(TBframe.CMD_DONE,'')
                        terminal['socket'].send(data)
                    elif type == TBframe.DEVICE_ERASE:
                        dst = value.split(',')
                        addr = (dst[0], int(dst[1]))
                        client = self.get_client_by_addr(addr)
                        if client == None:
                            data = TBframe.construct(TBframe.CMD_ERROR,'')
                            terminal['socket'].send(data)
                        else:
                            content = terminal['addr'][0]
                            content += ',' + str(terminal['addr'][1])
                            content += ',' + dst[2]
                            data = TBframe.construct(TBframe.DEVICE_ERASE, content)
                            client['socket'].send(data)
                    elif type == TBframe.DEVICE_PROGRAM:
                        dst = value.split(',')
                        addr = (dst[0], int(dst[1]))
                        client = self.get_client_by_addr(addr)
                        if client == None:
                            data = TBframe.construct(TBframe.CMD_ERROR,'')
                            terminal['socket'].send(data)
                        else:
                            content = terminal['addr'][0]
                            content += ',' + str(terminal['addr'][1])
                            content += ',' + dst[2]
                            content += ',' + dst[3]
                            content += ',' + filename[7:]
                            data = TBframe.construct(TBframe.DEVICE_PROGRAM, content)
                            client['socket'].send(data)
                    elif type == TBframe.DEVICE_RESET or type == TBframe.DEVICE_START or type == TBframe.DEVICE_STOP:
                        dst = (value.split(':')[0]).split(',')
                        addr = (dst[0], int(dst[1]))
                        client = self.get_client_by_addr(addr)
                        if client != None:
                            data = TBframe.construct(type, value)
                            client['socket'].send(data)
                    elif type == TBframe.DEVICE_CMD:
                        dst = (value.split(':')[0]).split(',')
                        addr = (dst[0], int(dst[1]))
                        client = self.get_client_by_addr(addr)
                        if client != None:
                            data = TBframe.construct(TBframe.DEVICE_CMD, value)
                            client['socket'].send(data)
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
                        if terminal['socket'] in client['devices'][port]:
                            continue
                        client['devices'][port].append(terminal['socket'])
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
                        if terminal['socket'] not in client['devices'][port]:
                            continue
                        client['devices'][port].remove(terminal['socket'])
                        print "terminal {0}:{1}".format(terminal['addr'][0], terminal['addr'][1]),
                        print "unsubscribed log of device {0}:{1}".format(values[0], port)
                    elif type == TBframe.LOG_DOWNLOAD:
                        values = value.split(',')
                        if len(values) != 3:
                            continue
                        addr = (values[0], int(values[1]))
                        port = values[2]
                        filename = 'server/' + addr[0] + '-' + port[5:] + '.log'
                        client = self.get_client_by_addr(addr)
                        if client == None or port not in list(client['devices']) or os.path.exists(filename) == False:
                            data = TBframe.construct(TBframe.CMD_ERROR,'')
                            terminal['socket'].send(data)
                            print "terminal {0}:{1}".format(terminal['addr'][0], terminal['addr'][1]),
                            print "downloading log of device {0}:{1} ...".format(values[0], port),
                            print "failed"
                            continue
                        self.send_file_to_someone(terminal, filename)
                        data = TBframe.construct(TBframe.CMD_DONE, '')
                        terminal['socket'].send(data)
                        print "terminal {0}:{1}".format(terminal['addr'][0], terminal['addr'][1]),
                        print "downloading log of device {0}:{1} ...".format(values[0], port),
                        print "succeed"
                    elif type == TBframe.HEARTBEAT:
                        heartbeat_timeout = time.time() + 30
            except socket.timeout:
                continue
            except:
                break
        for client in self.client_list:
            for port in list(client['devices']):
                if terminal['socket'] in client['devices'][port]:
                    client['devices'][port].remove(terminal['socket'])
        terminal['socket'].close()
        print "terminal ", terminal['addr'], "disconnected"
        self.terminal_list.remove(terminal)

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
            subprocess.call(['mkdir','server'])
        return "success"

    def run(self):
        try:
            client_thread = thread.start_new_thread(self.client_listen_thread, ())
            terminal_thread = thread.start_new_thread(self.terminal_listen_thread, ())
            while True:
                time.sleep(0.1)
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