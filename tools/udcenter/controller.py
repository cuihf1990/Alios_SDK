import os, sys, re, time, socket, ssl, signal, tty, termios
import select, thread, Queue, json, traceback
import sqlite3 as sql
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
        r, w, e = select.select(list(self.read_map), list(self.write_map), list(self.error_map))
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
        self.connections = {}
        self.timeouts = {}
        self.selector = None
        self.user_command = ''
        self.serve_funcs = {'none':    self.login_process, 'client':  self.client_process,
                            'server':  self.server_process, 'terminal':self.terminal_process}
        self.keyfile = 'server_key.pem'
        self.certfile = 'server_cert.pem'
        self.dbase = sql.connect('controller.db').cursor()
        self.dbase.execute('CREATE TABLE IF NOT EXISTS Users(key TEXT, devices TEXT)')

        if os.path.exists('controller') == False:
            try:
                os.mkdir('controller')
            except:
                print "create 'controller' directory failed"

        self.cmd_history = []
        if os.path.exists('controller/.cmd_history') == True:
            try:
                file = open('controller/.cmd_history','rb')
                self.cmd_history = json.load(file)
                file.close()
            except:
                print "read command history failed"

    def log_print(self, log):
        sys.stdout.write('\r' + log + '\r\n')
        sys.stdout.write('# ' + self.user_command)
        sys.stdout.flush()

    def accept(self, sock):
        conn, addr = sock.accept()
        self.log_print('{0} connected'.format(addr))
        conn.setblocking(False)
        self.selector.register(conn, selector.EVENT_READ, self.data_read)
        self.connections[conn] = {'role':'none', 'inbuff':'', 'outbuff':Queue.Queue(256)}
        self.timeouts[conn] = time.time() + 0.2

    def data_write(self, sock):
        queue = self.connections[sock]['outbuff']
        try:
            data = queue.get(False)
        except:
            pass
        else:
            #self.log_print(str(time.time()' + ' ' + data)
            sock.send(data)
        if queue.empty():
            self.selector.unregister(sock, selector.EVENT_WRITE)

    def data_read(self, sock):
        data = sock.recv(CONFIG_MAXMSG_LENTH)
        role = self.connections[sock]['role']
        if not data:
            self.log_print("{0} {1} disconnect".format(role, sock.getpeername()))
            self.selector.unregister(sock, selector.EVENT_READ)
            self.selector.unregister(sock, selector.EVENT_WRITE)
            sock.close()
            self.timeouts.pop(sock)
            self.connections.pop(sock)
            return
        msg = self.connections[sock]['inbuff']
        msg += data
        while msg != '':
            type, length, value, msg = TBframe.parse(msg)
            if type == TBframe.TYPE_NONE:
                break
            self.serve_funcs[role](sock, type, value)

    def send_data(self, sock, content):
        if sock not in self.connections:
            self.log_print("error: sending data to invalid connection {0}".format(sock.getpeername()))
            return False
        queue = self.connections[sock]['outbuff']
        if queue.full():
            self.log_print("warning: output buffer for {0} full, discard packet".format(sock.getpeername()))
            return False
        queue.put_nowait(content)
        self.selector.register(sock, selector.EVENT_WRITE, self.data_write)

    def choose_random_server(self):
        server_list = []
        for conn in self.connections:
            if self.connections[conn]['role'] != 'server':
                continue
            if 'valid' not in self.connections[conn]:
                continue
            if self.connections[conn]['valid'] == False:
                continue
            server_list.append(conn)
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

    def login_process(self, sock, type, value):
        if type != TBframe.ACCESS_LOGIN:
            content = TBframe.construct(TBframe.ACCESS_LOGIN, 'request')
            self.send_data(sock, content)
            return

        try:
            role = value.split(',')[0]
        except:
            role = None

        if role == None or role not in ['client', 'server', 'terminal']:
            content = TBframe.construct(TBframe.ACCESS_LOGIN, 'argerror')
            self.send_data(sock, content)
            return

        self.connections[sock]['role'] = role
        value = value[len(role)+1:]
        self.serve_funcs[role](sock, type, value)

    def client_process(self, sock, type, value):
        self.log_print('client {0} {1}'.format(type, value))
        self.timeouts[sock] = time.time() + CONFIG_TIMEOUT
        if type == TBframe.ACCESS_LOGIN:
            is_valid_uuid = re.match('^[0-9a-f]{12}$', value)
            server_sock = self.choose_random_server()
            if is_valid_uuid == None:
                content = TBframe.construct(TBframe.ACCESS_LOGIN, 'argerror')
                self.send_data(sock, content)
                return

            if server_sock == None:
                content = TBframe.construct(TBframe.ACCESS_LOGIN, 'noserver')
                self.send_data(sock, content)
                return

            uuid = value
            token = self.generate_random_token()
            content = '{0},{1}'.format(uuid, token)
            content = TBframe.construct(TBframe.ACCESS_ADD_CLIENT, content)
            self.send_data(server_sock, content)
            self.connections[sock]['uuid'] = uuid
            return

    def server_process(self, sock, type, value):
        self.log_print('server {0} {1}'.format(type, value))
        self.timeouts[sock] = time.time() + CONFIG_TIMEOUT
        if type == TBframe.ACCESS_LOGIN:
            try:
                ports = json.loads(value)
            except:
                if DEBUG: traceback.print_exc()
                ports = {}
            if 'client_port' not in ports or 'terminal_port' not in ports:
                content = TBframe.construct(TBframe.ACCESS_LOGIN, 'fail')
                self.connections[sock]['valid'] = False
            else:
                content = TBframe.construct(TBframe.ACCESS_LOGIN, 'ok')
                self.connections[sock]['client_port'] = ports['client_port']
                self.connections[sock]['terminal_port'] = ports['terminal_port']
                self.connections[sock]['valid'] = True
            self.send_data(sock, content)
            return
        if type == TBframe.ACCESS_REPORT_STATUS:
            try:
                status = json.loads(value)
            except:
                if DEBUG: traceback.print_exc()
                status = {}
            if not status:
                content = TBframe.construct(TBframe.ACCESS_REPORT_STATUS, 'fail')
            else:
                self.connections[sock]['status'] = status
                content = TBframe.construct(TBframe.ACCESS_REPORT_STATUS, 'ok')
            self.send_data(sock, content)
            return
        if type == TBframe.ACCESS_ADD_CLIENT:
            try:
                [ret, uuid, token] = value.split(',')
            except:
                if DEBUG: self.log_print("error: invalid return value {0}".format(repr(value)))
                return
            if ret != 'success':
                return #TODO: choose another server for the client

            client_sock = None
            for conn in self.connections:
                if self.connections[conn]['role'] != 'client':
                    continue
                if 'uuid' not in self.connections[conn]:
                    continue
                if self.connections[conn]['uuid'] != uuid:
                    continue
                client_sock = conn
                break
            if client_sock == None:
                return

            server_addr = sock.getpeername()[0]
            server_port = self.connections[sock]['client_port']
            content = 'ok,{0},{1},{2}'.format(server_addr, server_port, token)
            content = TBframe.construct(TBframe.ACCESS_LOGIN, content)
            self.send_data(client_sock, content)
            return

    def terminal_process(self, sock, type, value):
        self.log_print('terminal {0} {1}'.forma(type, value))
        if type == TBframe.ACCESS_LOGIN:
            return

    def sock_interact_thread(self, port):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.setblocking(False)
        sock.bind((self.host, port))
        if CONFIG_ENCRYPT:
            sock = ssl.wrap_socket(sock, self.keyfile, self.certfile, True)
        sock.listen(100)
        self.selector = selector()
        self.connections = {}
        self.timeouts = {}
        self.selector.register(sock, selector.EVENT_READ, self.accept)
        while self.keep_running:
            events = self.selector.select()
            for [sock, callback] in events:
                callback(sock)

            #close timeout connections
            now = time.time()
            for conn in list(self.timeouts):
                if now < self.timeouts[conn]:
                    continue
                self.log_print("{0} {1} timeout, close connection".format(role, conn.getpeername()))
                self.selector.unregister(conn, selector.EVENT_READ)
                self.selector.unregister(conn, selector.EVENT_WRITE)
                conn.close()
                self.timeouts.pop(conn)
                self.connections.pop(conn)

    def process_cmd(self, cmd):
        sys.stdout.write("cmd is {0}\r\n".format(repr(cmd)))
        return

    def user_interaction(self):
        cmd = self.user_command
        saved_cmd = ""
        history_index = -1
        escape = None
        p = 0
        old_settings = termios.tcgetattr(sys.stdin.fileno())
        tty.setraw(sys.stdin.fileno())
        sys.stdout.write("\r# " + cmd)
        sys.stdout.flush()
        while self.keep_running:
            try:
                c = sys.stdin.read(1)
            except:
                break
            #sys.stdout.write("\rkeycode {0}\r\n".format(ord(c)))
            if escape != None:
                escape += c
                if len(escape) == 2:
                    if ord(c) == 91:
                        continue
                    else:
                        escape = None

                if len(escape) == 3:
                    if ord(c) == 65: #KEY_UP
                        if history_index == -1:
                            saved_cmd = cmd
                        if history_index < (len(self.cmd_history) - 1):
                            history_index += 1
                        cmd = self.cmd_history[history_index]
                        sys.stdout.write("\r# " + " " * p)
                        p = len(cmd)
                        sys.stdout.write("\r# " + cmd)
                        sys.stdout.flush()
                        continue
                    if ord(c) == 66: #KEY_DOWN
                        if history_index <= -1:
                            history_index = -1
                            continue
                        history_index -= 1
                        if history_index >= 0:
                            cmd = self.cmd_history[history_index]
                        else:
                            cmd = saved_cmd
                        sys.stdout.write("\r# " + " " * p)
                        p = len(cmd)
                        sys.stdout.write("\r# " + cmd)
                        sys.stdout.flush()
                        continue
                    if ord(c) == 68: #KEY_LEFT
                        #if p <= 0:
                        #    continue
                        #p -= 1
                        #sys.stdout.write('\b')
                        #sys.stdout.flush()
                        continue
                    if ord(c) == 67: #KEY_RIGHT
                        #if p >= len(cmd):
                        #    continue
                        #p += 1
                        #sys.stdout.write('\x1c')
                        #sys.stdout.flush()
                        continue

            if ord(c) == 27: #ESCAPE
                escape = c
                continue
            if ord(c) == 13: #RETURN
                if cmd == "q" :
                    self.keep_running = False
                    time.sleep(0.2)
                    break
                elif cmd != "":
                    sys.stdout.write('\r# ' + cmd + '\r\n')
                    sys.stdout.flush()
                    self.process_cmd(cmd)
                    self.cmd_history = [cmd] + self.cmd_history
                cmd = ""
                saved_cmd = ""
                history_index = -1
                p = 0
                sys.stdout.write('# ')
                sys.stdout.flush()
                continue
            if c == '\x08' or c == '\x7f': #DELETE
                if cmd[0:p] == "":
                    continue
                newcmd = cmd[0:p-1] + cmd[p:]
                cmd = newcmd
                p -= 1
                sys.stdout.write('\b \b')
                sys.stdout.flush()
                continue
            if ord(c) == 3: #CTRL+C
                self.keep_running = False
                time.sleep(0.2)
                break

            try:
                newcmd = cmd[0:p] + c + cmd[p:]
                cmd = newcmd
                p += 1
                sys.stdout.write('\r# ' + cmd)
                sys.stdout.flush()
            except:
                traceback.print_exc()
                sys.stdout.write("\rError: unsupported unicode character {0}\n".format(c))
                sys.stdout.write("\r# " + cmd)
                sys.stdout.flush()
                continue
        try:
            if len(self.cmd_history) > 0:
                file = open("controller/.cmd_history",'wb')
                json.dump(self.cmd_history, file)
                file.close()
        except:
            print("error: save command history failed")
        termios.tcsetattr(sys.stdin.fileno(), termios.TCSADRAIN, old_settings)
        print ''
        return

    def run(self):
        signal.signal(signal.SIGINT, signal_handler)
        self.keep_running = True
        thread.start_new_thread(self.sock_interact_thread, (self.port,))
        self.user_interaction()

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
