import os, sys, re, time, socket, ssl, signal
import select, thread, Queue, json, traceback
import TBframe

CONFIG_TIMEOUT = 30
CONFIG_MAXMSG_LENTH = 8192
CONFIG_ENCRYPT = False
DEBUG = True

def signal_handler(sig, frame):
    print "received SIGINT"
    raise KeyboardInterrupt

class selector():
    EVENT_READ = 0
    EVENT_WRITE = 1
    EVENT_ERROR = 2
    def __init__(self):
        self.read_map = {}
        self.write_map = {}
        self.error_map = {}

    def register(self, fd, type, callback):
        types = {self.EVENT_READ:self.read_map, self.EVENT_WRITE:self.write_map, self.EVENT_ERROR: self.error_map}
        if type not in types:
            return
        map = types[type]
        map[fd] = callback

    def unregister(self, fd, type):
        types = {self.EVENT_READ:self.read_map, self.EVENT_WRITE:self.write_map, self.EVENT_ERROR: self.error_map}
        if type not in types:
            return
        map = types[type]
        if fd in map: map.pop(fd)

    def select(self):
        ret = []
        r, w, e = select.select(list(self.read_map), list(self.write_map), list(self.error_map), 0)
        for fd in r:
            ret.append([fd, self.read_map[fd]])
        for fd in w:
            ret.append([fd, self.write_map[fd]])
        for fd in e:
            ret.append([fd, self.error_map[fd]])
        return ret


class Controller():
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.clients = {}
        self.servers = {}
        self.terminals = {}
        self.client_sel = None
        self.server_sel = None
        self.terminal_sel = None
        self.timeouts = {}
        self.keyfile = 'server_key.pem'
        self.certfile = 'server_cert.pem'

    def accept(self, sel, sock, role):
        funcs = {'client':self.client_serve, 'server':self.server_serve, 'terminal':self.terminal_serve}
        clist = {'client':self.clients, 'server':self.servers, 'terminal':self.terminals}
        conn, addr = sock.accept()
        if role not in funcs:
            print "error: unsupported role {0}".format(role)
            conn.close()
            return
        print('{0} {1} connected'.format(role, addr))
        conn.setblocking(False)
        sel.register(conn, selector.EVENT_READ, funcs[role])
        self.timeouts[sel][conn] = time.time() + CONFIG_TIMEOUT
        clist[role][conn] = {'msg':'', 'buff':Queue.Queue(128)}

    def data_write(self, sel, sock, role):
        roles = {'client':self.clients, 'server':self.servers, 'terminal':self.terminals}
        queue = roles[role][sock]['buff']
        try:
            data = queue.get(False)
        except:
            pass
        else:
            #print time.time(), data
            sock.send(data)
        if queue.empty():
            sel.unregister(sock, selector.EVENT_WRITE)

    def send_data(self, sock, role, content):
        roles = {'client':self.clients, 'server':self.servers, 'terminal':self.terminals}
        selcs = {'client':self.client_sel, 'server':self.server_sel, 'terminal':self.terminal_sel}
        if sock not in roles[role]:
            print "error: {0} is not a {1}".format(sock.getpeername(), role)
            return False
        queue = roles[role][sock]['buff']
        if queue.full():
            return False
        queue.put_nowait(content)
        role_sel = selcs[role]
        role_sel.register(sock, selector.EVENT_WRITE, self.data_write)

    def choose_random_server(self):
        server_list = []
        for server in self.servers:
            if 'valid' not in self.servers[server]:
                continue
            if self.servers[server]['valid'] == False:
                continue
            server_list.append(server)
        if server_list == []:
            return None

        bytes = os.urandom(2)
        rand_num = ord(bytes[0]) * ord(bytes[1])
        rand_num = rand_num % len(server_list)
        return server_list[rand_num]

    def generate_random_token(self):
        bytes = os.urandom(8); token = ''
        for byte in bytes: token += '{0:02x}'.format(ord(byte))
        return token

    def client_serve(self, sel, sock, role):
        data = sock.recv(CONFIG_MAXMSG_LENTH)
        if not data:
            print("client {0} disconnect".format(sock.getpeername()))
            sel.unregister(sock, selector.EVENT_READ)
            sock.close()
            self.timeouts[sel].pop(sock)
            self.clients.pop(sock)
            return
        msg = self.clients[sock]['msg']
        msg += data
        while msg != '':
            type, length, value, msg = TBframe.parse(msg)
            if type == TBframe.TYPE_NONE:
                break
            print role, type, value
            self.timeouts[sel][sock] = time.time() + CONFIG_TIMEOUT
            if type == TBframe.ACCESS_LOGIN:
                is_valid_uuid = re.match('^[0-9a-f]{12}$', value)
                server_sock = self.choose_random_server()
                if is_valid_uuid and server_sock:
                    uuid = value
                    token = self.generate_random_token()
                    content = '{0},{1}'.format(uuid, token)
                    content = TBframe.construct(TBframe.ACCESS_ADD_CLIENT, content)
                    self.send_data(server_sock, 'server', content)
                    self.clients[sock]['uuid'] = uuid
                else:
                    content = TBframe.construct(TBframe.ACCESS_LOGIN, 'fail')
                    self.send_data(sock, role, content)
                continue

    def server_serve(self, sel, sock, role):
        data = sock.recv(CONFIG_MAXMSG_LENTH)
        if not data:
            print("server {0} disconnect".format(sock.getpeername()))
            sel.unregister(sock, selector.EVENT_READ)
            sock.close()
            self.timeouts[sel].pop(sock)
            self.servers.pop(sock)
            return
        msg = self.servers[sock]['msg']
        msg += data
        while msg != '':
            type, length, value, msg = TBframe.parse(msg)
            if type == TBframe.TYPE_NONE:
                break
            print role, type, value
            self.timeouts[sel][sock] = time.time() + CONFIG_TIMEOUT
            if type == TBframe.ACCESS_LOGIN:
                try:
                    ports = json.loads(value)
                except:
                    if DEBUG: traceback.print_exc()
                    ports = {}
                if 'client_port' not in ports or 'terminal_port' not in ports:
                    content = TBframe.construct(TBframe.ACCESS_LOGIN, 'fail')
                    self.servers[sock]['valid'] = False
                else:
                    content = TBframe.construct(TBframe.ACCESS_LOGIN, 'ok')
                    self.servers[sock]['client_port'] = ports['client_port']
                    self.servers[sock]['terminal_port'] = ports['terminal_port']
                    self.servers[sock]['valid'] = True
                self.send_data(sock, role, content)
                continue
            if type == TBframe.ACCESS_REPORT_STATUS:
                try:
                    status = json.loads(value)
                except:
                    if DEBUG: traceback.print_exc()
                    status = {}
                if not status:
                    content = TBframe.construct(TBframe.ACCESS_REPORT_STATUS, 'fail')
                else:
                    self.servers[sock]['status'] = status
                    content = TBframe.construct(TBframe.ACCESS_REPORT_STATUS, 'ok')
                self.send_data(sock, role, content)
                continue
            if type == TBframe.ACCESS_ADD_CLIENT:
                try:
                    [ret, uuid, token] = value.split(',')
                except:
                    if DEBUG: print "error: invalid return value {0}".format(repr(value))
                    continue
                if ret != 'success':
                    continue #TODO: choose another server for the client

                client_sock = None
                for c_sock in self.clients:
                    if 'uuid' not in self.clients[c_sock]:
                        continue
                    if self.clients[c_sock]['uuid'] != uuid:
                        continue
                    client_sock = c_sock
                    break
                if client_sock == None:
                    continue

                server_addr = sock.getpeername()[0]
                server_port = self.servers[sock]['client_port']
                content = 'ok,{0},{1},{2}'.format(server_addr, server_port, token)
                content = TBframe.construct(TBframe.ACCESS_LOGIN, content)
                self.send_data(client_sock, 'client', content)

    def terminal_serve(self, sel, sock, role):
        data = sock.recv(CONFIG_MAXMSG_LENTH)
        if not data:
            print("terminal {0} disconnect".format(sock.getpeername()))
            sel.unregister(sock, selector.EVENT_READ)
            sock.close()
            self.timeouts[sel].pop(sock)
            self.terminals.pop(sock)
            return
        self.timeouts[sel][sock] = time.time() + CONFIG_TIMEOUT
        print role, data

    def service_thread(self, port, role):
        roles = {'client':self.clients, 'server':self.servers, 'terminal':self.terminals}
        if role not in roles:
            return
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.setblocking(False)
        sock.bind((self.host, port))
        if CONFIG_ENCRYPT:
            sock = ssl.wrap_socket(sock, self.keyfile, self.certfile, True)
        sock.listen(100)
        sel = selector()
        if role == 'client': self.client_sel = sel
        if role == 'server': self.server_sel = sel
        if role == 'terminal': self.terminal_sel = sel
        sel.register(sock, selector.EVENT_READ, self.accept)
        self.timeouts[sel] = {}
        while self.keep_running:
            events = sel.select()
            for [sock, callback] in events:
                callback(sel, sock, role)

            #close timeout connections
            now = time.time()
            if role == 'client': conlist = self.clients
            if role == 'server': conlist = self.servers
            if role == 'terminal': sconlist = self.terminals
            for conn in list(self.timeouts[sel]):
                if now < self.timeouts[sel][conn]:
                    continue
                sel.unregister(conn, selector.EVENT_READ)
                conn.close()
                self.timeouts[sel].pop(conn)
                conlist.pop(conn)

    def run(self):
        signal.signal(signal.SIGINT, signal_handler)
        self.keep_running = True
        thread.start_new_thread(self.service_thread, (self.port, 'client',))
        thread.start_new_thread(self.service_thread, (self.port + 2, 'server',))
        thread.start_new_thread(self.service_thread, (self.port + 1, 'terminal',))
        while True:
            try:
                time.sleep(1)
            except:
                self.keep_running = False
                break

if __name__ == '__main__':
    host = ''
    port = '34567'
    if len(sys.argv) > 1:
        host = sys.argv[1]
    if len(sys.argv) > 2:
        port = sys.argv[2]
    cntr = Controller(host, 34567)
    cntr.run()
    sys.exit(0)
