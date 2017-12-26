import os, re, time, thread, traceback

class emulator:
    def __init__(self, port):
        self.port = port
        self.tag = ''

        #generate mac address
        self.macaddr = ''
        bytes = os.urandom(6)
        for byte in bytes:
            self.macaddr += '{0:02x}-'.format(ord(byte))

        #generate uuid
        bytes = os.urandom(16)
        if ord(bytes[0]) < 51:
            self.uuid = 'alink is not connected'
        else:
            self.uuid = 'uuid: '
            for byte in bytes:
                self.uuid += '{0:02X}'.format(byte)
        self.macaddr = self.macaddr[:-1]
        self.kernel_version = 'AOS-R-1.1.2'
        self.app_version = 'app-1.1.0-' + time.strftime("%Y%m%d.%H%M")
        self.ser = None
        try:
            import serial
        except:
            print 'error: pyserial is NOT installed'
            return
        self.ser = serial.Serial(port, 115200, timeout = 0.01)
        self.running = False
        self.builtin_cmds = ['help', 'devname', 'mac', 'version', 'uuid', 'umesh']
        self.umesh_cmds = ['help', 'status', 'nbrs']

    def put_line(self, content):
        self.ser.write(self.tag + content + '\r\n')

    def cmd_process(self, args):
        PROMPT = self.tag + '\r\n# '
        self.put_line(' '.joint(args))
        if args[0] not in self.builtin_cmds:
            self.put_line('unkonwn command: {0}'.format(' '.joing(args)))
            self.ser.write(PROMPT)
            return

        if args[0] == 'help':
            self.put_line('buildin commands:')
            for cmd in self.builtin_cmds:
                self.put_line(cmd)
            self.ser.write(PROMPT)
            return
        if args[0] == 'devname':
            self.put_line('emulator')
            self.ser.write(PROMPT)
            return
        if args[0] == 'mac':
            self.put_line(self.macaddr)
            self.ser.write(PROMPT)
            return
        if args[0] == 'version':
            self.put_line('kernel version :' + self.kernel_version)
            self.put_line('app versin :' + self.app_version)
            self.ser.write(PROMPT)
            return
        if args[0] == 'uuid':
            self.put_line(self.uuid)
            self.ser.write(PROMPT)
            return
        if args[0] == 'umesh':
            if len(args) < 2 or args[1] not in self.umesh_cmds:
                self.put_line('')
                self.ser.write(PROMPT)
                return
            if args[1] == 'help':
                for cmd in self.umesh_cmds:
                    self.put_line(cmd)
                self.ser.write(PROMPT)
                return
            if args[1] == 'status':
                self.put_line('state\tdetatched')
                self.put_line('\tattach\tidle')
                self.put_line('<<network wifi 0>>')
                self.put_line('\tnetid\t0xfffe')
                self.put_line('\tmac\t' + self.macaddr.replace('-', ''))
                self.put_line('\tsid\t0xfffe')
                self.put_line('\tnetsize\t0')
                self.put_line('\trouter\tSID_ROUTER')
                self.put_line('\tbcast_mut\t512')
                self.put_line('\tucast_mut\t512')
                self.put_line('\tuptime\t' + str(int(time.time() - self.start_time)))
                self.put_line('\tchannel\t0')
                self.ser.write(PROMPT)
                return
            if args[1] == 'nbrs':
                self.put_line('\t<<hal type wifi>>')
                self.put_line('\tnum=0')
                self.ser.write(PROMPT)
                return

    def main_loop(self):
        self.start_time = time.time()
        tag_seq = re.compile(r"\x1b\[t[0-9a-f]+m")
        cmd = ''
        try:
            self.ser.write('# ')
        except:
            self.running = False
            return
        while self.running and os.path.exists(self.port):
            try:
                c = self.ser.read(1)
                if c == '':
                    continue
                elif c != '\r' or c != '\n':
                    cmd += c
                    continue
            except:
                if os.path.exists(self.port) == False:
                    break
                traceback.print_exc()

            if cmd == '':
                self.ser.write('\r\n')
                continue
            tag_match = tag_seq.match(cmd)
            if tag_math == None:
                self.tag = ''
            else:
                self.tag = cmd[tag_match.start():tag_match.end()]
                cmd = tag_seq.sub('', cmd)
            if cmd == '':
                self.ser.write('error: empty command\r\n')
                continue
            args = cmd.split(cmd)
            self.cmd_process(args)
        self.running = False

    def start(self):
        if self.ser == None:
            return False
        thread.start_new_thread(self.main_loop, ())

    def stop(self):
        if self.running == False:
            return False
        self.running = False
        time.sleep(0.1)
        return True
