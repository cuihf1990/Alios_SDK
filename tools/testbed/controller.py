import sys, socket, ssl, asyncore, signal
import TBframe

MAX_MSG_LENTH = 10240
ENCRYPT = True

def signal_handler(sig, frame):
    print "received SIGINT"
    raise KeyboardInterrupt

class ServiceHandler(asyncore.dispatcher):
    def __init__(self, sock, addr, clients, servers, terminals):
        self.addr = addr
        self.buffer = ''
        self.msg = ''
        self.mode = None
        self.clients = clients
        self.servers = servers
        self.terminals = terminals
        asyncore.dispatcher.__init__(self, sock)

    def writable(self):
        return (self.buffer != '')

    def handle_read(self):
        new_msg = self.recv(8192)
        if new_msg == '':
            return
        print new_msg
        self.msg += new_msg
        while self.msg != '':
            type, length, value, self.msg = TBframe.parse(self.msg)
            if type == TBframe.TYPE_NONE:
                break
            print type, value
            if self.mode == None:
                self.login_message_process(type, value)
            elif self.mode == 'client':
                self.client_message_process(type, value)
            elif self.mode == 'server':
                self.server_message_process(type, value)
            elif self.mode == 'terminal':
                self.terminal_message_process(type, value)

    def handle_write(self):
        sent = self.send(self.buffer)
        self.buffer = self.buffer[sent:]

    def send_data(self, data):
        self.buffer += data

    def login_message_process(self, type, value):
        print 'login_process'
        if type not in [TBframe.CLIENT_LOGIN, TBframe.SERVER_LOGIN, TBframe.TERMINAL_LOGIN]:
            content = TBframe.construct(TBframe.REQUEST_LOGIN, 'please login')
            self.buffer += content
            return

        if type == TBframe.CLIENT_LOGIN:
            self.clients[self.addr] = {'socket':self.socket}
            self.mode = 'client'
            return
        if type == TBframe.SERVER_LOGIN:
            self.servers[self.addr] = {'socket':self.socket}
            self.mode = 'server'
            return
        if type == TBframe.TERMINAL_LOGIN:
            self.terminals[self.addr] = {'socket':self.socket}
            self.mode = 'terminal'
            return

    def client_message_process(self, type, value):
        print 'client_process'
        print type, value

    def server_message_process(self, type, value):
        print 'server_process'
        print type, value

    def terminal_message_process(self, type, value):
        print 'terminal_process'
        print type, value

class Controller(asyncore.dispatcher):
    def __init__(self, host, port):
        self.clients = {}
        self.servers = {}
        self.terminals = {}
        self.keyfile = 'server_key.pem'
        self.certfile = 'server_cert.pem'
        asyncore.dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM, ENCRYPT)
        self.set_reuse_addr()
        self.bind((host, port))
        self.listen(5)

    def create_socket(self, family, type, encrypt):
        self.family_and_type = family, type
        sock = socket.socket(family, type)
        if encrypt:
            sock = ssl.wrap_socket(sock, self.keyfile, self.certfile, True)
        sock.setblocking(0)
        self.set_socket(sock)

    def handle_accept(self):
        pair = self.accept()
        if pair is not None:
            sock, addr = pair
            print('Incoming connection from {0}'.format(addr))
            ServiceHandler(sock, addr, self.clients, self.servers, self.terminals)

    def run(self):
        signal.signal(signal.SIGINT, signal_handler)
        try:
            asyncore.loop()
        except:
            pass

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
