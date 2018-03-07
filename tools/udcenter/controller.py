import os, sys, re, time, socket, ssl, signal, tty, termios
import select, thread, Queue, json, traceback
import sqlite3 as sql
import packet as pkt

CONFIG_TIMEOUT = 30
CONFIG_MAXMSG_LENTH = 8192
ENCRYPT = True
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
        self.user_cmd = ''
        self.cur_pos = 0
        self.models = {'mk3060':'mk3060-', 'esp32':'esp32-'}
        self.serve_funcs = {'none':    self.login_process, 'client':  self.client_process,
                            'server':  self.server_process, 'terminal':self.terminal_process}
        self.keyfile = 'controller_key.pem'
        self.certfile = 'controller_certificate.pem'
        self.dbase = sql.connect('controller.db', check_same_thread = False)
        #sqlcmd = 'CREATE TABLE IF NOT EXISTS Users(uuid TEXT, name TEXT, email TEXT, devices TEXT)' #v0.1
        sqlcmd = 'CREATE TABLE IF NOT EXISTS Users(uuid TEXT, name TEXT, info TEXT)' #v0.2
        self.database_excute_sqlcmd(sqlcmd)

        sqlcmd = 'CREATE TABLE IF NOT EXISTS Clients(uuid TEXT, name TEXT, info TEXT)'
        self.database_excute_sqlcmd(sqlcmd)

        sqlcmd = 'CREATE TABLE IF NOT EXISTS Devices(device TEXT, uuid TEXT, timeout REAL)'
        self.database_excute_sqlcmd(sqlcmd)

        sqlcmd = 'SELECT * FROM Devices'
        rows = self.database_excute_sqlcmd(sqlcmd)
        self.devices = {}
        if rows == None:
            print "database error: polling Devices table failed"
        else:
            for row in rows:
                try:
                    self.devices[row[0]] = {'uuid': row[1], 'timeout': row[2]}
                except:
                    print "database error detected while initializing"

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

    def database_excute_sqlcmd(self, sqlcmd):
        with self.dbase:
            cur = self.dbase.cursor()
            try:
                cur.execute(sqlcmd)
            except:
                #traceback.print_exc()
                ret = None
            else:
                ret = cur.fetchall()
        return ret

    def netlog_print(self, log):
        if len(self.user_cmd) >= len(log):
            sys.stdout.write('\r' + ' ' * len(self.user_cmd))
        sys.stdout.write('\r' + log + '\r\n')
        self.cmd_print()

    def accept(self, sock):
        try:
            conn, addr = sock.accept()
        except:
            return
        self.netlog_print('{0} connected'.format(addr))
        conn.setblocking(False)
        self.selector.register(conn, selector.EVENT_READ, self.data_read)
        self.connections[conn] = {'role':'none', 'addr': addr, 'inbuff':'', 'outbuff':Queue.Queue(256)}
        self.timeouts[conn] = time.time() + 0.2

    def data_write(self, sock):
        if sock not in self.connections:
            return
        queue = self.connections[sock]['outbuff']
        try:
            data = queue.get(False)
        except:
            pass
        else:
            #self.netlog_print(str(time.time()' + ' ' + data)
            sock.send(data)
        if queue.empty():
            self.selector.unregister(sock, selector.EVENT_WRITE)

    def data_read(self, sock):
        try:
            data = sock.recv(CONFIG_MAXMSG_LENTH)
        except:
            data = None
        role = self.connections[sock]['role']
        if not data:
            addr = self.connections[sock]['addr']
            self.netlog_print("{0} {1} disconnect".format(role, addr))
            self.selector.unregister(sock, selector.EVENT_READ)
            self.selector.unregister(sock, selector.EVENT_WRITE)
            sock.close()
            self.timeouts.pop(sock)
            self.connections.pop(sock)
            return
        msg = self.connections[sock]['inbuff']
        msg += data
        while msg != '':
            type, length, value, msg = pkt.parse(msg)
            if type == pkt.TYPE_NONE:
                break
            self.serve_funcs[role](sock, type, value)

    def send_data(self, sock, content):
        if sock not in self.connections:
            try:
                self.netlog_print("error: sending data to invalid connection {0}".format(sock.getpeername()))
            except:
                self.netlog_print("error: sending data to invalid connection {0}".format(repr(sock)))
            return False
        queue = self.connections[sock]['outbuff']
        if queue.full():
            addr = self.connections[sock]['addr']
            self.netlog_print("warning: output buffer for {0} full, discard packet".format(addr))
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

    def generate_random_hexstr(self, len):
        bytes = os.urandom(int(round(len/2.0))); hexstr = ''
        for byte in bytes: hexstr += '{0:02x}'.format(ord(byte))
        return hexstr[:len]

    def login_process(self, sock, type, value):
        if type != pkt.ACCESS_LOGIN:
            content = pkt.construct(pkt.ACCESS_LOGIN, 'request')
            self.send_data(sock, content)
            return

        try:
            role = value.split(',')[0]
        except:
            role = None

        if role == None or role not in ['client', 'server', 'terminal']:
            content = pkt.construct(pkt.ACCESS_LOGIN, 'argerror')
            self.send_data(sock, content)
            return

        self.connections[sock]['role'] = role
        self.serve_funcs[role](sock, type, value)

    def client_process(self, sock, type, value):
        self.netlog_print('client {0} {1}'.format(type, value))
        if type == pkt.ACCESS_LOGIN:
            if value.startswith('client,'):
                uuid = value[len('client,'):]
                is_valid_uuid = re.match('^[0-9a-f]{16}$', uuid)
                server_sock = self.choose_random_server()
            else:
                is_valid_uuid = None
            if is_valid_uuid == None:
                content = pkt.construct(pkt.ACCESS_LOGIN, 'invalid access key')
                self.send_data(sock, content)
                return

            sqlcmd = "SELECT * FROM Clients WHERE uuid = '{0}'".format(uuid)
            ret = self.database_excute_sqlcmd(sqlcmd)
            if ret == None or len(ret) != 1:
                content = pkt.construct(pkt.ACCESS_LOGIN, 'invalid access key')
                self.send_data(sock, content)
                return

            if server_sock == None:
                content = pkt.construct(pkt.ACCESS_LOGIN, 'noserver')
                self.send_data(sock, content)
                return

            token = self.generate_random_hexstr(16)
            content = '{0},{1}'.format(uuid, token)
            content = pkt.construct(pkt.ACCESS_ADD_CLIENT, content)
            self.send_data(server_sock, content)
            self.connections[sock]['uuid'] = uuid
            self.timeouts[sock] = time.time() + CONFIG_TIMEOUT
            return

    def server_process(self, sock, type, value):
        self.timeouts[sock] = time.time() + CONFIG_TIMEOUT
        if type in [pkt.HEARTBEAT, pkt.ACCESS_UPDATE_TERMINAL, pkt.ACCESS_DEL_TERMINAL, pkt.ACCESS_DEL_CLIENT]:
            return
        self.netlog_print('server {0} {1}'.format(type, value))
        if type == pkt.ACCESS_LOGIN:
            if value.startswith('server,'):
                value = value[len('server,'):]
                try:
                    server_info = json.loads(value)
                except:
                    server_info = {}
            else:
                server_info = {}

            fields = ['uuid', 'client_port', 'terminal_port', 'certificate']
            is_valid_server = True
            for info in fields:
                if info in server_info:
                    continue
                is_valid_server = False
                break
            if is_valid_server and ENCRYPT and server_info['certificate'] == 'None':
                is_valid_server = False

            if is_valid_server == False:
                content = pkt.construct(pkt.ACCESS_LOGIN, 'fail')
                self.connections[sock]['valid'] = False
            else:
                content = pkt.construct(pkt.ACCESS_LOGIN, 'ok')
                self.connections[sock]['uuid'] = server_info['uuid']
                self.connections[sock]['client_port'] = server_info['client_port']
                self.connections[sock]['terminal_port'] = server_info['terminal_port']
                if ENCRYPT:
                    self.connections[sock]['certificate'] = server_info['certificate']
                else:
                    self.connections[sock]['certificate'] = 'None'
                self.connections[sock]['valid'] = True
            self.send_data(sock, content)
            return
        if type == pkt.ACCESS_REPORT_STATUS:
            try:
                status = json.loads(value)
            except:
                if DEBUG: traceback.print_exc()
                status = {}
            if not status:
                content = pkt.construct(pkt.ACCESS_REPORT_STATUS, 'fail')
            else:
                self.connections[sock]['status'] = status
                content = pkt.construct(pkt.ACCESS_REPORT_STATUS, 'ok')
            self.send_data(sock, content)
            return
        if type == pkt.ACCESS_ADD_CLIENT:
            try:
                [ret, uuid, token] = value.split(',')
            except:
                if DEBUG: self.netlog_print("error: invalid return value {0}".format(repr(value)))
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

            server_addr = self.connections[sock]['addr'][0]
            server_port = self.connections[sock]['client_port']
            certificate = self.connections[sock]['certificate']
            content = 'success,{0},{1},{2},{3}'.format(server_addr, server_port, token, certificate)
            content = pkt.construct(pkt.ACCESS_LOGIN, content)
            self.send_data(client_sock, content)
            return
        if type == pkt.ACCESS_ADD_TERMINAL:
            try:
                [ret, uuid, token] = value.split(',')
            except:
                if DEBUG: self.netlog_print("error: invalid return value {0}".format(repr(value)))
                return

            terminal_sock = None
            for conn in self.connections:
                if self.connections[conn]['role'] != 'terminal':
                    continue
                if 'uuid' not in self.connections[conn]:
                    continue
                if self.connections[conn]['uuid'] != uuid:
                    continue
                terminal_sock = conn
                break
            if terminal_sock == None:
                return

            if ret != 'success':
                content = pkt.construct(pkt.ACCESS_LOGIN, 'fail')
                self.send_data(terminal_sock, content)
                return

            server_addr = self.connections[sock]['addr'][0]
            server_port = self.connections[sock]['terminal_port']
            certificate = self.connections[sock]['certificate']
            content = 'success,{0},{1},{2},{3}'.format(server_addr, server_port, token, certificate)
            content = pkt.construct(pkt.ACCESS_LOGIN, content)
            self.send_data(terminal_sock, content)
            return

    def find_server_for_target(self, target, type):
        type = type + 's'
        for conn in self.connections:
            if self.connections[conn]['role'] != 'server':
                continue
            if self.connections[conn]['valid'] == False:
                continue
            if type not in self.connections[conn]['status']:
                continue
            if target not in self.connections[conn]['status'][type]:
                continue
            return conn
        return None

    def terminal_process(self, sock, type, value):
        self.netlog_print('terminal {0} {1}'.format(type, value))
        if type == pkt.ACCESS_LOGIN:
            if value.startswith('terminal,'):
                uuid = value[len('terminal,'):]
                is_valid_uuid = re.match('^[0-9a-f]{16}$', uuid)
            else:
                is_valid_uuid = None
            if is_valid_uuid == None:
                content = pkt.construct(pkt.ACCESS_LOGIN, 'invalid access key')
                self.send_data(sock, content)
                return

            sqlcmd = "SELECT * FROM Users WHERE uuid = '{0}'".format(uuid)
            ret = self.database_excute_sqlcmd(sqlcmd)
            if ret == None or len(ret) != 1:
                content = pkt.construct(pkt.ACCESS_LOGIN, 'invalid access key')
                self.send_data(sock, content)
                return

            devices = []; client_uuids = []; server_sock = None
            for device in self.devices:
                if self.devices[device]['uuid'] != uuid:
                    continue
                devices.append(device)
                client_uuid = device[0:16]
                if client_uuid in client_uuids:
                    continue
                client_uuids.append(client_uuid)
                if server_sock != None:
                    continue
                server_sock = self.find_server_for_target(client_uuid, 'client')
            self.netlog_print("{0} {1}".format(client_uuids, server_sock))
            if devices == []:
                content = pkt.construct(pkt.ACCESS_LOGIN, 'no allocated device')
                self.send_data(sock, content)
                return
            if server_sock == None:
                content = pkt.construct(pkt.ACCESS_LOGIN, 'allocated devices not connected')
                self.send_data(sock, content)
                return

            token = self.generate_random_hexstr(16)
            devices = '|'.join(devices)
            content = '{0},{1},{2}'.format(uuid, token, devices)
            content = pkt.construct(pkt.ACCESS_ADD_TERMINAL, content)
            self.send_data(server_sock, content)
            self.connections[sock]['uuid'] = uuid
            return

    def sock_interact_thread(self, port):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.setblocking(False)
        sock.bind((self.host, port))
        if ENCRYPT:
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
                role = self.connections[conn]['role']
                addr = self.connections[conn]['addr']
                self.netlog_print("{0} {1} timeout, close connection".format(role, addr))
                self.selector.unregister(conn, selector.EVENT_READ)
                self.selector.unregister(conn, selector.EVENT_WRITE)
                conn.close()
                self.timeouts.pop(conn)
                self.connections.pop(conn)

    def house_keeping_thread(self):
        while self.keep_running:
            time.sleep(1)

            #remove timeouted devices
            now = time.time()
            for device in list(self.devices):
                if now <= self.devices[device]['timeout']:
                    continue
                uuid = self.devices[device]['uuid']
                self.devices.pop(device)
                sqlcmd = "DELETE FROM Devices WHERE device = '{0}'".format(device)
                ret = self.database_excute_sqlcmd(sqlcmd)
                if ret == None:
                    self.cmdlog_print("warning: delete device {0} from Devices database failed".format(device))
                self.inform_server_updates(uuid, 'update_terminal')

    def cmdlog_print(self, log):
        sys.stdout.write(log + '\r\n')

    def inform_server_updates(self, uuid, type):
        types = ['delete_terminal', 'delete_client', 'update_terminal']
        if type not in types:
            return
        if 'terminal' in type:
            conn = self.find_server_for_target(uuid, 'terminal')
        elif 'client' in type:
            conn = self.find_server_for_target(uuid, 'client')
        else:
            conn = None
        if conn == None:
            return

        if type == 'delete_terminal':
            content = pkt.construct(pkt.ACCESS_DEL_TERMINAL, uuid)
        if type == 'delete_client':
            content = pkt.construct(pkt.ACCESS_DEL_CLIENT, uuid)
        elif type == 'update_terminal':
            devices = []
            for device in self.devices:
                if self.devices[device]['uuid'] != uuid:
                    continue
                devices.append(device)
            devices = '|'.join(devices)
            content = pkt.construct(pkt.ACCESS_UPDATE_TERMINAL, uuid + ',' + devices)
        self.send_data(conn, content)

    def add_cmd_handler(self, args):
        if len(args) < 2:
            self.cmdlog_print('usages: add user name [info1 info2 info3 ... infoN]')
            self.cmdlog_print('        add client name [info1 info2 info3 ... infoN]')
            self.cmdlog_print('        add device uuid device1|device2|...|deviceN [days]')
            return

        if args[0] in ['user', 'client']:
            Tables = {'user':'Users', 'client':'Clients'}
            uuid = self.generate_random_hexstr(16)
            name = args[1]
            info = ' '.join(args[2:])
            table = Tables[args[0]]
            sqlcmd = "INSERT INTO {0} VALUES('{1}', '{2}', '{3}')".format(table, uuid, name, info)
            ret = self.database_excute_sqlcmd(sqlcmd)
            if ret == None:
                self.cmdlog_print("add {0} '{1}' failed".format(args[0], name))
            else:
                self.cmdlog_print("add {0} '{1}' succeed, uuid={2}".format(args[0], name, uuid))
        elif args[0] == 'device':
            if len(args) < 3:
                self.cmdlog_print('usage error, usage: add device uuid devices [days]')
                return
            uuid = args[1]
            dev_str = args[2]
            if len(args) > 3:
                try:
                    days = float(args[3])
                except:
                    self.cmdlog_print('error: invalid input {0}'.format(repr(args[3])))
                    return
            else:
                days = 7

            timeout = time.time() + 3600 * 24 * days

            sqlcmd = "SELECT * FROM Users WHERE uuid = '{0}'".format(uuid)
            rows = self.database_excute_sqlcmd(sqlcmd)
            if len(rows) < 1:
                self.cmdlog_print("error: user '{0}' does not exist".format(uuid))
                return

            devs = dev_str.split('|')
            devices = []
            for dev in devs:
                if re.match("^[0-9a-f]{16}:.", dev) == None:
                    self.cmdlog_print("error: invalid device {0}".format(dev))
                    return
                if dev in self.devices and self.devices[dev]['uuid'] != uuid:
                    self.cmdlog_print("error: device {0} already alloated".format(dev))
                    return
                devices += [dev]
            for device in devices:
                if device not in self.devices:
                    sqlcmd = "INSERT INTO Devices VALUES('{0}', '{1}', '{2}')".format(device, uuid, timeout)
                    ret = self.database_excute_sqlcmd(sqlcmd)
                else:
                    sqlcmd = "UPDATE Devices SET timeout = '{0}' WHERE device = '{1}'".format(timeout, device)
                    ret = self.database_excute_sqlcmd(sqlcmd)
                if ret == None:
                    self.cmdlog_print("add device '{0}' failed: error adding device to database".format(device))
                    return
                self.devices[device] = {'uuid': uuid, 'timeout': timeout}

            self.cmdlog_print("succeed")
            self.inform_server_updates(uuid, 'update_terminal')
        else:
            self.cmdlog_print("error: invalid command option {0}".format(repr(args[0])))
        return

    def del_cmd_handler(self, args):
        if len(args) < 2:
            self.cmdlog_print('usage: del user uuid')
            self.cmdlog_print('       del client uuid')
            self.cmdlog_print('       del device uuid device1|device2|...|deviceN')
            return

        option = args[0]
        uuid   = args[1]
        if len(args) > 2:
            dev_str = args[2]
        else:
            dev_str = None
        if option == 'client':
            sqlcmd = "SELECT * FROM Clients WHERE uuid = '{0}'".format(uuid)
            rows = self.database_excute_sqlcmd(sqlcmd)
            if len(rows) < 1:
                self.cmdlog_print("error: client '{0}' does not exist".format(uuid))
                return
        else:
            sqlcmd = "SELECT * FROM Users WHERE uuid = '{0}'".format(uuid)
            rows = self.database_excute_sqlcmd(sqlcmd)
            if len(rows) < 1:
                self.cmdlog_print("error: user '{0}' does not exist".format(uuid))
                return

        if option == 'user':
            sqlcmd = "DELETE FROM Devices WHERE uuid = '{0}'".format(uuid)
            ret = self.database_excute_sqlcmd(sqlcmd)
            if ret == None:
                self.cmdlog_print("warning: delete devices of {0} from database failed".format(uuid))
            for device in list(self.devices):
                if self.devices[device]['uuid'] != uuid:
                    continue
                self.devices.pop(device)
            sqlcmd = "DELETE FROM Users WHERE uuid = '{0}'".format(uuid)
            ret = self.database_excute_sqlcmd(sqlcmd)
            if ret == None:
                self.cmdlog_print("failed")
            else:
                self.cmdlog_print("succeed")
            self.inform_server_updates(uuid, 'delete_terminal')
            return
        elif option == 'client':
            sqlcmd = "DELETE FROM Clients WHERE uuid = '{0}'".format(uuid)
            ret = self.database_excute_sqlcmd(sqlcmd)
            if ret == None:
                self.cmdlog_print("failed")
            else:
                self.cmdlog_print("succeed")
            self.inform_server_updates(uuid, 'delete_client')
            return
        elif option == 'device':
            if dev_str == None:
                self.cmdlog_print("error: please input the devices you want to delete")
                return

            del_devices = dev_str.split('|')
            for device in del_devices:
                if re.match("^[0-9a-f]{12,16}:.", device) == None:
                    self.cmdlog_print("error: invalid device {0}".format(device))
                    return
                if device not in self.devices or self.devices[device]['uuid'] != uuid:
                    self.cmdlog_print("error: user {0} does not own device {1} ".format(uuid, device))
                    return
            if del_devices == []:
                return
            for device in del_devices:
                sqlcmd = "DELETE FROM Devices WHERE device = '{0}'".format(device)
                ret = self.database_excute_sqlcmd(sqlcmd)
                if ret == None:
                    self.cmdlog_print("warning: delete device {0} from database failed".format(device))
                self.devices.pop(device)
            self.cmdlog_print("succeed")
            self.inform_server_updates(uuid, 'update_terminal')
            return
        else:
            self.cmdlog_print('usage error, invalid argument {0}'.format(repr(option)))
            return
        return

    def allocate_cmd_handler(self, args):
        if len(args) < 3:
            self.cmdlog_print('usage: allocate uuid nubmer model [days]')
            return

        uuid = args[0]
        number = args[1]
        model = args[2]
        if len(args) > 3:
            try:
                days = float(args[3])
            except:
                self.cmdlog_print('error: invalid input {0}'.format(repr(args[3])))
                return
        else:
            days = 7

        sqlcmd = "SELECT * FROM Users WHERE uuid = '{0}'".format(uuid)
        rows = self.database_excute_sqlcmd(sqlcmd)
        if len(rows) < 1:
            self.cmdlog_print("error: user '{0}' does not exist".format(uuid))
            return

        try:
            number = int(number)
        except:
            number = 0
        if number <= 0:
            self.cmdlog_print('error: invalid input {0}, input a positive integer'.format(args[1]))
            return

        if model not in self.models:
            self.cmdlog_print('error: unsupported model {0}'.format(model))
            return
        model_path_str = self.models[model]

        ext_server = None
        ext_devices = []
        for device in self.devices:
            if self.devices[device]['uuid'] != uuid:
                continue
            ext_devices.append(device)
            conn = self.find_server_for_target(device, 'device')
            if conn == None:
                continue
            ext_server = conn
            break
        if ext_devices != [] and ext_server == None:
            self.cmdlog_print('error: can not locate the exist server for {0}'.format(uuid))
            return
        if ext_server == None:
            allocated = []
            for conn in self.connections:
                if self.connections[conn]['role'] != 'server':
                    continue
                if self.connections[conn]['valid'] == False:
                    continue
                if 'devices' not in self.connections[conn]['status']:
                    continue
                allocated = []
                for dev in self.connections[conn]['status']['devices']:
                    if model_path_str not in dev:
                        continue
                    if dev in self.devices:
                        continue
                    allocated.append(dev)
                    if len(allocated) >= number:
                        break
                if len(allocated) >= number:
                    break
        else:
            allocated = []
            for dev in self.connections[ext_server]['status']['devices']:
                if model_path_str not in dev:
                    continue
                if dev in self.devices:
                    continue
                allocated.append(dev)
                if len(allocated) >= number:
                    break

        if len(allocated) < number:
            self.cmdlog_print('failed')
            return
        timeout = time.time() + 3600 * 24 * days
        for device in allocated:
            sqlcmd = "INSERT INTO Devices VALUES('{0}', '{1}', '{2}')".format(device, uuid, timeout)
            ret = self.database_excute_sqlcmd(sqlcmd)
            if ret == None:
                self.cmdlog_print("add device '{0}' failed: error adding device to database".format(device))
                return
            self.devices[device] = {'uuid': uuid, 'timeout': timeout}
        self.cmdlog_print('succeed, allocat: {0}'.format('|'.join(allocated)))
        self.inform_server_updates(uuid, 'update_terminal')

    def print_user(self, uuid=None):
        if uuid == None:
            sqlcmd = "SELECT * FROM Users"
        else:
            sqlcmd = "SELECT * FROM Users where uuid = '{0}'".format(uuid)

        rows = self.database_excute_sqlcmd(sqlcmd)

        if rows == None:
            self.cmdlog_print("error: poll database failed")
            return

        if uuid == None:
            user_num = len(rows)
            self.cmdlog_print("users({0}):".format(user_num))
            for row in rows:
                key = row[0]; name = row[1]; info=row[2]
                devices = []
                for device in self.devices:
                    if self.devices[device]['uuid'] != key:
                        continue
                    devices.append(device)
                device_num = len(devices)
                if self.find_server_for_target(key, 'terminal') == None:
                    status = 'offline'
                else:
                    status = 'online'
                self.cmdlog_print("|--{0} ({1} {2}) {3} {4}".format(key, device_num, status, name, info))
                for device in devices:
                    try:
                        timeout = time.strftime("%Y-%m-%d@%H:%M:%S", time.localtime(self.devices[device]['timeout']))
                    except:
                        timeout = 'None'
                    self.cmdlog_print("|  |--{0} valid till {1}".format(device, timeout))
        else:
            if len(rows) == 0:
                self.cmdlog_print("error: uuid {0} does not exist in database".format(repr(uuid)))
                return
            self.cmdlog_print("users:")
            for row in rows:
                key = row[0]; name = row[1]; info=row[2]
                devices = []
                for device in self.devices:
                    if self.devices[device]['uuid'] != key:
                        continue
                    devices.append(device)
                device_num = len(devices)
                if self.find_server_for_target(key, 'terminal') == None:
                    status = 'offline'
                else:
                    status = 'online'
                self.cmdlog_print("|--{0} ({1} {2}) {3} {4}".format(key, device_num, status, name, info))
                for device in devices:
                    self.cmdlog_print("|  |--{0}".format(device))

    def print_server(self, uuid=None):
        self.cmdlog_print("servers:")
        tab = '  |'
        for conn in self.connections:
            if self.connections[conn]['valid'] == False:
                continue
            if self.connections[conn]['role'] != 'server':
                continue
            if uuid != None and uuid != self.connections[conn]['uuid']:
                continue
            server_uuid = self.connections[conn]['uuid']
            server_port = self.connections[conn]['addr'][1]
            self.cmdlog_print("|" + tab*0 + '--' + '{0}-{1}:'.format(server_uuid, server_port))
            if 'clients' not in self.connections[conn]['status']:
                continue
            terminal_num = len(self.connections[conn]['status']['terminals'])
            self.cmdlog_print("|" + tab*1 + '--' + 'terminals({0}):'.format(terminal_num))
            for terminal in self.connections[conn]['status']['terminals']:
                self.cmdlog_print('|' + tab*2 + '--' + terminal)
            if len(self.connections[conn]['status']['clients']) == 0:
                continue
            client_num = len(self.connections[conn]['status']['clients'])
            self.cmdlog_print("|" + tab*1 + '--' + 'clients({0}):'.format(client_num))
            for client in self.connections[conn]['status']['clients']:
                self.cmdlog_print('|' + tab*2 + '--' + client)
            if len(self.connections[conn]['status']['devices']) == 0:
                continue
            device_num = len(self.connections[conn]['status']['devices'])
            self.cmdlog_print("|" + tab*1 + '--' + 'devices({0}):'.format(device_num))
            for device in self.connections[conn]['status']['devices']:
                self.cmdlog_print('|     |--' + device)

    def print_client(self):
        sqlcmd = "SELECT * FROM Clients"
        rows = self.database_excute_sqlcmd(sqlcmd)
        if rows == None:
            self.cmdlog_print("error: poll database failed")
            return

        num = len(rows)
        self.cmdlog_print("clients({0}):".format(num))
        for row in rows:
            key = row[0]; name = row[1]; info=row[2]
            if self.find_server_for_target(key, 'client') == None:
                status = 'offline'
            else:
                status = 'online'
            self.cmdlog_print("|--{0} ({1}) {2} {3}".format(key, status, name, info))

    def list_cmd_handler(self, args):
        if len(args) == 0:
            self.print_user(None)
            self.cmdlog_print('')
            self.print_server()
            self.cmdlog_print('')
            self.print_client()

        if len(args) >= 1:
            if args[0] == 'user':
                uuid = None
                if len(args) >= 2:
                    uuid = args[1]
                self.print_user(uuid)
            elif args[0] == 'server':
                uuid = None
                if len(args) >= 2:
                    uuid = args[1]
                self.print_server(uuid)
            elif args[0] == 'client':
                self.print_client()
        return

    def help_cmd_handler(self, args):
        self.cmdlog_print("Usages:")
        self.cmdlog_print("    add user name info")
        self.cmdlog_print("    add client name info")
        self.cmdlog_print("    add device uuid device1|deice2|...|deviceN [days(7)]")
        self.cmdlog_print("    del user uuid")
        self.cmdlog_print("    del device uuid devic1|device2|...|deviceN")
        self.cmdlog_print("    alloc uuid number model")
        self.cmdlog_print("    list")
        self.cmdlog_print("    list user [uuid]")
        self.cmdlog_print("    list server [uuid]")
        return

    def process_cmd(self):
        cmds = self.user_cmd.split()
        if cmds[0] == 'add':
            self.add_cmd_handler(cmds[1:])
        elif cmds[0] == 'del':
            self.del_cmd_handler(cmds[1:])
        elif cmds[0] == 'alloc':
            self.allocate_cmd_handler(cmds[1:])
        elif cmds[0] == 'list':
            self.list_cmd_handler(cmds[1:])
        elif cmds[0] == 'help':
            self.help_cmd_handler(cmds[1:])
        else:
            sys.stdout.write("unknow command {0}\r\n".format(repr(self.user_cmd)))
        sys.stdout.flush()
        return

    def cmd_print(self):
        cmd_str = '\r# ' + ' ' * (len(self.user_cmd) + 1)
        cmd_str += '\r# ' + self.user_cmd
        cmd_str += '\b' * (len(self.user_cmd) - self.cur_pos)
        sys.stdout.write(cmd_str)
        sys.stdout.flush()

    def user_interaction(self):
        self.user_cmd = ''
        self.cur_pos = 0
        saved_cmd = ""
        history_index = -1
        escape = None
        old_settings = termios.tcgetattr(sys.stdin.fileno())
        tty.setraw(sys.stdin.fileno())
        self.cmd_print()
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
                            saved_cmd = self.user_cmd
                        if history_index < (len(self.cmd_history) - 1):
                            history_index += 1
                        sys.stdout.write("\r  " + " " * len(self.user_cmd))
                        self.user_cmd = self.cmd_history[history_index]
                        self.cur_pos = len(self.user_cmd)
                        self.cmd_print()
                        continue
                    if ord(c) == 66: #KEY_DOWN
                        if history_index <= -1:
                            history_index = -1
                            continue
                        history_index -= 1
                        sys.stdout.write("\r  " + " " * len(self.user_cmd))
                        if history_index >= 0:
                            self.user_cmd = self.cmd_history[history_index]
                        else:
                            self.user_cmd = saved_cmd
                        self.cur_pos = len(self.user_cmd)
                        self.cmd_print()
                        continue
                    if ord(c) == 68: #KEY_LEFT
                        if self.cur_pos <= 0:
                            continue
                        self.cur_pos -= 1
                        self.cmd_print()
                        continue
                    if ord(c) == 67: #KEY_RIGHT
                        if self.cur_pos >= len(self.user_cmd):
                            continue
                        self.cur_pos += 1
                        self.cmd_print()
                        continue

            if ord(c) == 27: #ESCAPE
                escape = c
                continue
            if ord(c) == 13: #RETURN
                if self.user_cmd == "q" :
                    self.keep_running = False
                    time.sleep(0.2)
                    break
                elif self.user_cmd != "":
                    sys.stdout.write('\r# ' + self.user_cmd + '\r\n')
                    sys.stdout.flush()
                    self.process_cmd()
                    self.cmd_history = [self.user_cmd] + self.cmd_history
                self.user_cmd = ""
                saved_cmd = ""
                history_index = -1
                self.cur_pos = 0
                self.cmd_print()
                continue
            if c == '\x08' or c == '\x7f': #DELETE
                if self.user_cmd[0:self.cur_pos] == "":
                    continue
                newcmd = self.user_cmd[0:self.cur_pos-1] + self.user_cmd[self.cur_pos:]
                self.user_cmd = newcmd
                self.cur_pos -= 1
                self.cmd_print()
                continue
            if ord(c) == 3: #CTRL+C
                self.keep_running = False
                time.sleep(0.2)
                break

            try:
                newcmd = self.user_cmd[0:self.cur_pos] + c + self.user_cmd[self.cur_pos:]
                self.user_cmd = newcmd
                self.cur_pos += 1
                self.cmd_print()
            except:
                traceback.print_exc()
                sys.stdout.write("\rError: unsupported unicode character {0}\n".format(c))
                self.cmd_print()
                continue
        termios.tcsetattr(sys.stdin.fileno(), termios.TCSADRAIN, old_settings)
        try:
            if len(self.cmd_history) > 0:
                file = open("controller/.cmd_history",'wb')
                json.dump(self.cmd_history, file)
                file.close()
        except:
            print("error: save command history failed")
        print ''
        return

    def run(self):
        signal.signal(signal.SIGINT, signal_handler)
        self.keep_running = True
        thread.start_new_thread(self.sock_interact_thread, (self.port,))
        thread.start_new_thread(self.house_keeping_thread, ())
        self.user_interaction()
