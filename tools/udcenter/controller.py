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
        self.user_cmd = ''
        self.cur_pos = 0
        self.serve_funcs = {'none':    self.login_process, 'client':  self.client_process,
                            'server':  self.server_process, 'terminal':self.terminal_process}
        self.keyfile = 'server_key.pem'
        self.certfile = 'server_cert.pem'
        self.dbase = sql.connect('controller.db', check_same_thread = False)
        sqlcmd = 'CREATE TABLE IF NOT EXISTS Users(uuid TEXT, name TEXT, email TEXT, devices TEXT)'
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
        ret = None
        with self.dbase:
            cur = self.dbase.cursor()
            try:
                cur.execute(sqlcmd)
            except:
                traceback.print_exc()
            else:
                ret = cur.fetchall()
        return ret



    def netlog_print(self, log):
        if len(self.user_cmd) >= len(log):
            sys.stdout.write('\r' + ' ' * len(self.user_cmd))
        sys.stdout.write('\r' + log + '\r\n')
        self.cmd_print()

    def accept(self, sock):
        conn, addr = sock.accept()
        self.netlog_print('{0} connected'.format(addr))
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
            #self.netlog_print(str(time.time()' + ' ' + data)
            sock.send(data)
        if queue.empty():
            self.selector.unregister(sock, selector.EVENT_WRITE)

    def data_read(self, sock):
        data = sock.recv(CONFIG_MAXMSG_LENTH)
        role = self.connections[sock]['role']
        if not data:
            self.netlog_print("{0} {1} disconnect".format(role, sock.getpeername()))
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
            self.netlog_print("error: sending data to invalid connection {0}".format(sock.getpeername()))
            return False
        queue = self.connections[sock]['outbuff']
        if queue.full():
            self.netlog_print("warning: output buffer for {0} full, discard packet".format(sock.getpeername()))
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
        self.serve_funcs[role](sock, type, value)

    def client_process(self, sock, type, value):
        self.netlog_print('client {0} {1}'.format(type, value))
        if type == TBframe.ACCESS_LOGIN:
            if value.startswith('client,'):
                uuid = value[len('client,'):]
                is_valid_uuid = re.match('^[0-9a-f]{12}$', uuid)
                server_sock = self.choose_random_server()
            else:
                is_valid_uuid = None
            if is_valid_uuid == None:
                content = TBframe.construct(TBframe.ACCESS_LOGIN, 'argerror')
                self.send_data(sock, content)
                return

            if server_sock == None:
                content = TBframe.construct(TBframe.ACCESS_LOGIN, 'noserver')
                self.send_data(sock, content)
                return

            token = self.generate_random_hexstr(16)
            content = '{0},{1}'.format(uuid, token)
            content = TBframe.construct(TBframe.ACCESS_ADD_CLIENT, content)
            self.send_data(server_sock, content)
            self.connections[sock]['uuid'] = uuid
            self.timeouts[sock] = time.time() + CONFIG_TIMEOUT
            return

    def server_process(self, sock, type, value):
        self.timeouts[sock] = time.time() + CONFIG_TIMEOUT
        if type in [TBframe.HEARTBEAT, TBframe.ACCESS_UPDATE_TERMINAL, TBframe.ACCESS_DEL_TERMINAL]:
            return
        self.netlog_print('server {0} {1}'.format(type, value))
        if type == TBframe.ACCESS_LOGIN:
            if value.startswith('server,'):
                value = value[len('server,'):]
                try:
                    info = json.loads(value)
                except:
                    info = {}
            else:
                info = {}
            if 'uuid' not in info or 'client_port' not in info or 'terminal_port' not in info:
                content = TBframe.construct(TBframe.ACCESS_LOGIN, 'fail')
                self.connections[sock]['valid'] = False
            else:
                content = TBframe.construct(TBframe.ACCESS_LOGIN, 'ok')
                self.connections[sock]['uuid'] = info['uuid']
                self.connections[sock]['client_port'] = info['client_port']
                self.connections[sock]['terminal_port'] = info['terminal_port']
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

            server_addr = sock.getpeername()[0]
            server_port = self.connections[sock]['client_port']
            content = 'success,{0},{1},{2}'.format(server_addr, server_port, token)
            content = TBframe.construct(TBframe.ACCESS_LOGIN, content)
            self.send_data(client_sock, content)
            return
        if type == TBframe.ACCESS_ADD_TERMINAL:
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
                content = TBframe.construct(TBframe.ACCESS_LOGIN, 'fail')
                self.send_data(terminal_sock, content)
                return

            server_addr = sock.getpeername()[0]
            server_port = self.connections[sock]['terminal_port']
            content = 'success,{0},{1},{2}'.format(server_addr, server_port, token)
            content = TBframe.construct(TBframe.ACCESS_LOGIN, content)
            self.send_data(terminal_sock, content)
            return

    def get_server_by_client_uuid(self, uuid):
        for sock in self.connections:
            if self.connections[sock]['role'] != 'server':
                continue
            if self.connections[sock]['valid'] == False:
                continue
            if 'status' not in self.connections[sock]:
                continue
            if 'clients' not in self.connections[sock]['status']:
                continue
            if uuid not in self.connections[sock]['status']['clients']:
                continue
            return sock
        return None

    def terminal_process(self, sock, type, value):
        self.netlog_print('terminal {0} {1}'.format(type, value))
        if type == TBframe.ACCESS_LOGIN:
            if value.startswith('terminal,'):
                uuid = value[len('terminal,'):]
                is_valid_uuid = re.match('^[0-9a-f]{16}$', uuid)
            else:
                is_valid_uuid = None
            if is_valid_uuid == None:
                content = TBframe.construct(TBframe.ACCESS_LOGIN, 'argerror')
                self.send_data(sock, content)
                return

            sqlcmd = "SELECT * FROM Users WHERE uuid = '{0}'".format(uuid)
            ret = self.database_excute_sqlcmd(sqlcmd)
            if ret == None or len(ret) != 1:
                content = TBframe.construct(TBframe.ACCESS_LOGIN, 'invaliduuid')
                self.send_data(sock, content)
                return

            devices = ret[0][3]
            if devices == '':
                content = TBframe.construct(TBframe.ACCESS_LOGIN, 'nodevice')
                self.send_data(sock, content)
                return
            client_uuid = devices.split(':')[0]
            server_sock = self.get_server_by_client_uuid(client_uuid)
            if server_sock == None:
                content = TBframe.construct(TBframe.ACCESS_LOGIN, 'not connected')
                self.send_data(sock, content)
                return

            token = self.generate_random_hexstr(16)
            content = '{0},{1},{2}'.format(uuid, token, devices)
            content = TBframe.construct(TBframe.ACCESS_ADD_TERMINAL, content)
            self.send_data(server_sock, content)
            self.connections[sock]['uuid'] = uuid
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
                role = self.connections[conn]['role']
                self.netlog_print("{0} {1} timeout, close connection".format(role, conn.getpeername()))
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
                sqlcmd = "SELECT * FROM Users WHERE uuid = '{0}'".format(uuid)
                ret = self.database_excute_sqlcmd(sqlcmd)
                if ret == None:
                    self.cmdlog_print("warning: database error detected while deleting device {0}".format(device))
                    continue
                devs = ret[0][3].split('|')
                if device not in devs:
                    continue
                devs.remove(device)
                devs = '|'.join(devs)
                sqlcmd = "UPDATE Users SET devices = '{0}' WHERE uuid = '{1}'".format(devs, uuid)
                ret = self.database_excute_sqlcmd(sqlcmd)
                if ret == None:
                    self.cmdlog_print("warning: delete device {0} from Users database failed".format(device))
                conn = self.find_server_for_target(uuid, 'terminal')
                if conn == None:
                    continue
                content = TBframe.construct(TBframe.ACCESS_UPDATE_TERMINAL, uuid + ',' + devs)
                self.send_data(conn, content)

    def cmdlog_print(self, log):
        sys.stdout.write(log + '\r\n')

    def find_server_for_target(self, target, type):
        type = type + 's'
        for conn in self.connections:
            if self.connections[conn]['valid'] == False:
                continue
            if self.connections[conn]['role'] != 'server':
                continue
            if type not in self.connections[conn]['status']:
                continue
            if target not in self.connections[conn]['status'][type]:
                continue
            return conn
        return None

    def add_cmd_handler(self, args):
        if len(args) < 2:
            self.cmdlog_print('usages: add user name [email] [device1|device2|...|deviceN] [days]')
            self.cmdlog_print('        add device uuid device1|device2|...|deviceN [days]')
            return
        if args[0] == 'user':
            if len(args) < 2:
                self.cmdlog_print('usage error, usage: add user name [email] [devices] [days]')
                return
            uuid = self.generate_random_hexstr(16)
            name = args[1]
            if len(args) > 2:
                email = args[2]
            else:
                email = 'None'
            if len(args) > 3:
                dev_str = args[3]
            else:
                dev_str = ''
            if len(args) > 4:
                try:
                    days = float(args[4])
                except:
                    self.cmdlog_print('error: invalid input {0}'.format(repr(args[4])))
                    return
            else:
                days = 7
            devs = dev_str.split('|')
            timeout = time.time() + 3600 * 24 * days
            devices = []
            for dev in devs:
                if re.match("^[0-9a-f]{12}:.", dev) == None:
                    self.cmdlog_print("error: invalid device {0}".format(dev))
                    continue
                devices += [dev]

            for device in devices:
                if device not in self.devices:
                    sqlcmd = "INSERT INTO Devices VALUES('{0}', '{1}', '{2}')".format(device, uuid, timeout)
                    ret = self.database_excute_sqlcmd(sqlcmd)
                else:
                    sqlcmd = "UPDATE Devices SET timeout = '{0}' WHERE device = '{1}'".format(timeout, device)
                    ret = self.database_excute_sqlcmd(sqlcmd)
                if ret == None:
                    self.cmdlog_print("add user '{0}' failed: error adding device to database".format(name))
                    return
                self.devices[device] = {'uuid': uuid, 'timeout': timeout}

            devices = '|'.join(devices)
            sqlcmd = "INSERT INTO Users VALUES('{0}', '{1}', '{2}', '{3}')".format(uuid, name, email, devices)
            ret = self.database_excute_sqlcmd(sqlcmd)
            if ret == None:
                self.cmdlog_print("add user '{0}' failed".format(name))
            else:
                self.cmdlog_print("add user '{0}' success, email: {1}, uuid: {2}, devices: {3}".format(name, email, uuid, devices))

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
                self.cmdlog_print("error: invalid uuid '{0}'".format(uuid))
                return
            if len(rows) > 1:
                self.cmdlog_print("database error: multiple record for single user")
                return
            ext_devices = rows[0][3].split('|')

            devs = dev_str.split('|')
            devices = []
            for dev in devs:
                if re.match("^[0-9a-f]{12}:.", dev) == None:
                    self.cmdlog_print("error: invalid device {0}".format(dev))
                    continue
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
                if device not in ext_devices:
                    ext_devices += [device]

            devices = '|'.join(ext_devices)
            sqlcmd = "UPDATE Users SET devices = '{0}' WHERE uuid = '{1}'".format(devices, uuid)
            ret = self.database_excute_sqlcmd(sqlcmd)
            if ret == None:
                self.cmdlog_print("failed")
            else:
                self.cmdlog_print("succeed")
            conn = self.find_server_for_target(uuid, 'terminal')
            if conn == None:
                return
            content = TBframe.construct(TBframe.ACCESS_UPDATE_TERMINAL, uuid + ',' + devices)
            self.send_data(conn, content)
        else:
            self.cmdlog_print("error: invalid command {0}".format(self.user_cmd))
        return

    def del_cmd_handler(self, args):
        if len(args) < 1:
            self.cmdlog_print('usage error, usage: del uuid [devices]')
            return
        uuid = args[0]
        if len(args) > 1:
            dev_str = args[1]
        else:
            dev_str = None
        sqlcmd = "SELECT * FROM Users WHERE uuid = '{0}'".format(uuid)
        rows = self.database_excute_sqlcmd(sqlcmd)
        if len(rows) < 1:
            self.cmdlog_print("error: invalid uuid '{0}'".format(uuid))
        devices = rows[0][3].split('|')
        if dev_str == None:
            del_devices = devices
        else:
            del_devices = []
            devs = dev_str.split('|')
            for dev in devs:
                if re.match("^[0-9a-f]{12}:.", dev) == None:
                    self.cmdlog_print("warning: invalid device {0}".format(dev))
                    continue
                if dev not in devices:
                    self.cmdlog_print("warning: device {0} none exist".format(dev))
                    continue
                del_devices += [dev]
                devices.remove(dev)
        for device in del_devices:
            sqlcmd = "DELETE FROM Devices WHERE device = '{0}'".format(device)
            ret = self.database_excute_sqlcmd(sqlcmd)
            if ret == None:
                self.cmdlog_print("warning: delete device {0} from database failed".format(device))
            self.devices.pop(device)
        if dev_str == None:
            sqlcmd = "DELETE FROM Users WHERE uuid = '{0}'".format(uuid)
        else:
            devices = '|'.join(devices)
            sqlcmd = "UPDATE Users SET devices = '{0}' WHERE uuid = '{1}'".format(devices, uuid)
        ret = self.database_excute_sqlcmd(sqlcmd)
        if ret == None:
            self.cmdlog_print("failed")
        else:
            self.cmdlog_print("succeed")
        conn = self.find_server_for_target(uuid, 'terminal')
        if conn == None:
            return
        if dev_str == None:
            content = TBframe.construct(TBframe.ACCESS_DEL_TERMINAL, uuid)
        else:
            content = TBframe.construct(TBframe.ACCESS_UPDATE_TERMINAL, uuid + ',' + devices)
        self.send_data(conn, content)
        return

    def print_user(self, uuid=None):
        if uuid == None:
            sqlcmd = "SELECT * FROM Users"
        else:
            sqlcmd = "SELECT * FROM Users where uuid = {0}".format(uuid)

        rows = self.database_excute_sqlcmd(sqlcmd)

        if uuid == None:
            self.cmdlog_print("users:")
            for row in rows:
                key = row[0]; name = row[1]; email=row[2]
                devices = row[3].split('|')
                self.cmdlog_print("|--{0} {1} {2}".format(key, name, email))
                for device in devices:
                    try:
                        timeout = time.strftime("%Y-%m-%d@%H:%M:%S", time.localtime(self.devices[device]['timeout']))
                    except:
                        timeout = 'None'
                    self.cmdlog_print("|  |--{0} valid till {1}".format(device, timeout))
        else:
            if len(rows) == 0:
                self.cmdlog_print("error: uuid {0} does not exist in database".format(repr(uuid)))
            self.cmdlog_print("users:")
            for row in rows:
                key = row[0]; name = row[1]; email=row[2]
                devices = row[3].split('|')
                self.cmdlog_print("|--{0} {1} {2}".format(key, name, email))
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
            server_port = conn.getpeername()[1]
            self.cmdlog_print("|" + tab*0 + '--' + '{0}-{1}:'.format(server_uuid, server_port))
            if 'clients' not in self.connections[conn]['status']:
                continue
            self.cmdlog_print("|" + tab*1 + '--' + 'terminals:')
            for terminal in self.connections[conn]['status']['terminals']:
                self.cmdlog_print('|' + tab*2 + '--' + terminal)
            if len(self.connections[conn]['status']['clients']) == 0:
                continue
            self.cmdlog_print("|" + tab*1 + '--' + 'clients:')
            for client in self.connections[conn]['status']['clients']:
                self.cmdlog_print('|' + tab*2 + '--' + client)
            if len(self.connections[conn]['status']['devices']) == 0:
                continue
            self.cmdlog_print("|" + tab*1 + '--' + 'devices:')
            for device in self.connections[conn]['status']['devices']:
                self.cmdlog_print('|     |--' + device)

    def list_cmd_handler(self, args):
        if len(args) == 0:
            self.print_user(None)
            self.cmdlog_print('')
            self.print_server()

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
        return

    def help_cmd_handler(self, args):
        self.cmdlog_print("Usages:")
        self.cmdlog_print("       add user name [email] [device1|deice2|...|deviceN] [days(7)]")
        self.cmdlog_print("       add device uuid device1|deice2|...|deviceN [days(7)]")
        self.cmdlog_print("       del uuid [devices]")
        self.cmdlog_print("       list")
        self.cmdlog_print("       list user [uuid]")
        self.cmdlog_print("       list server [uuid]")
        return

    def process_cmd(self):
        cmds = self.user_cmd.split()
        if cmds[0] == 'add':
            self.add_cmd_handler(cmds[1:])
        elif cmds[0] == 'del':
            self.del_cmd_handler(cmds[1:])
        elif cmds[0] == 'list':
            self.list_cmd_handler(cmds[1:])
        elif cmds[0] == 'help':
            self.help_cmd_handler(cmds[1:])
        else:
            sys.stdout.write("unknow command {0}\r\n".format(repr(self.user_cmd)))
        sys.stdout.flush()
        return

    def cmd_print(self):
        cmd_str = '\r# ' + self.user_cmd
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
                sys.stdout.write('\b \b')
                sys.stdout.flush()
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
