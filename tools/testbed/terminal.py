#!/usr/bin/python

import os, sys, time, curses, socket
import subprocess, thread, threading, pickle
from operator import itemgetter
import TBframe

MAX_MSG_LENTH = 2000
CMD_WINDOW_HEIGHT = 2
DEV_WINDOW_WIDTH  = 32
LOG_WINDOW_HEIGHT = 30
LOG_WINDOW_WIDTH  = 80

class Terminal:
    def __init__(self):
        self.stdscr = 0
        self.log_window = 0
        self.dev_window = 0
        self.cmd_window = 0
        self.keep_running = True
        self.device_list= []
        self.service_socket = 0
        self.cmd_excute_state = 'idle'
        self.log_content = []
        self.log_subscribed = []
        self.max_log_width = 0
        self.cur_color_pair = 7
        self.alias_tuples = {}
        self.cmd_history = []
        self.last_runcmd_dev = []

    def init(self, server_ip, server_port):
        #connect to server
        self.service_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.service_socket.connect((server_ip, server_port))
        except:
            print "connect to server {0}:{1} failed".format(server_ip, server_port)
            return "connect failed"
        if os.path.exists('terminal') == False:
            subprocess.call(['mkdir','terminal'])

        if os.path.exists('terminal/.alias') == True:
            try:
                file = open('terminal/.alias','rb')
                self.alias_tuples = pickle.load(file)
                file.close()
            except:
                print "read alias record failed"

        if os.path.exists('terminal/.cmd_history') == True:
            try:
                file = open('terminal/.cmd_history','rb')
                self.cmd_history = pickle.load(file)
                file.close()
            except:
                print "read alias record failed"

        #initialize UI
        try:
            self.stdscr = curses.initscr()
            curses.noecho()
            curses.cbreak()
            curses.start_color()
            curses.use_default_colors()
            curses.init_pair(0, curses.COLOR_BLACK, -1)
            curses.init_pair(1, curses.COLOR_RED, -1)
            curses.init_pair(2, curses.COLOR_GREEN, -1)
            curses.init_pair(3, curses.COLOR_YELLOW, -1)
            curses.init_pair(4, curses.COLOR_BLUE, -1)
            curses.init_pair(5, curses.COLOR_MAGENTA, -1)
            curses.init_pair(6, curses.COLOR_CYAN, -1)
            curses.init_pair(7, curses.COLOR_WHITE, -1)
            self.curseslock = threading.Lock();
            self.window_redraw()
            return "success"
        except:
            return "UI failed"

    def window_redraw(self):
        global LOG_WINDOW_WIDTH
        global LOG_WINDOW_HEIGHT
        self.curseslock.acquire()
        (max_y, max_x) = self.stdscr.getmaxyx()
        for y in range(max_y):
            for x in range(max_x - 1):
                self.stdscr.addch(y, x, ' ')
        self.stdscr.refresh()
        LOG_WINDOW_HEIGHT = max_y - CMD_WINDOW_HEIGHT - 3
        LOG_WINDOW_WIDTH = max_x - DEV_WINDOW_WIDTH - 3
        if len(self.log_content) > (LOG_WINDOW_HEIGHT - 1):
            self.log_content = self.log_content[-(LOG_WINDOW_HEIGHT - 1):]
        if self.max_log_width > (LOG_WINDOW_WIDTH - 1):
            self.log_content = []
        width = 1 + LOG_WINDOW_WIDTH + 1 + DEV_WINDOW_WIDTH + 1
        height = 1 + LOG_WINDOW_HEIGHT + 1 + CMD_WINDOW_HEIGHT + 1
        horline = '-' * (width-2)
        self.stdscr.addstr(0, 1, horline)
        self.stdscr.addstr(LOG_WINDOW_HEIGHT + 1, 1, horline)
        self.stdscr.addstr(LOG_WINDOW_HEIGHT + CMD_WINDOW_HEIGHT + 2, 1, horline)
        for i in range(1, height-1):
            self.stdscr.addch(i, 0, '|')
        for i in range(1, height - CMD_WINDOW_HEIGHT - 2):
            self.stdscr.addch(i, LOG_WINDOW_WIDTH + 1, '|')
        for i in range(1, height-1):
            self.stdscr.addch(i, LOG_WINDOW_WIDTH + DEV_WINDOW_WIDTH + 2, '|')
        self.stdscr.refresh()
        x = 1; y = 1;
        self.log_window = curses.newwin(LOG_WINDOW_HEIGHT, LOG_WINDOW_WIDTH, y, x)
        x = 1 + LOG_WINDOW_WIDTH + 1; y = 1;
        self.dev_window = curses.newwin(LOG_WINDOW_HEIGHT, DEV_WINDOW_WIDTH, y, x)
        x = 1; y = 1 + LOG_WINDOW_HEIGHT + 1;
        self.cmd_window = curses.newwin(CMD_WINDOW_HEIGHT, LOG_WINDOW_WIDTH + 1 + DEV_WINDOW_WIDTH, y, x)
        self.cmd_window.keypad(1)
        self.log_window.addstr(0, 0, "Logs", curses.A_BOLD)
        self.log_window.move(1, 0)
        self.log_window.refresh()
        self.dev_window.addstr(0, 0, "Devices", curses.A_BOLD)
        self.dev_window.move(1, 0)
        self.dev_window.refresh()
        self.cmd_window.addstr(0, 0, "Command:", curses.A_BOLD)
        self.cmd_window.refresh()
        self.curseslock.release()

    def process_esc_sequence(self, escape):
        colors = escape[2:-1].split(';')
        self.cur_color_pair = 7
        for color in colors:
            if color == "30":
                self.cur_color_pair = 0
            elif color == "31":
                self.cur_color_pair = 1
            elif color == "32":
                self.cur_color_pair = 2
            elif color == "33":
                self.cur_color_pair = 3
            elif color == "34":
                self.cur_color_pair = 4
            elif color == "35":
                self.cur_color_pair = 5
            elif color == "36":
                self.cur_color_pair = 6
            elif color == "37":
                self.cur_color_pair = 7

    def log_display(self, logtime, log):
        self.log_window.move(1,0)
        clear = (' ' * (LOG_WINDOW_WIDTH - 1) + '\n') * (LOG_WINDOW_HEIGHT - 2)
        clear += ' ' * (LOG_WINDOW_WIDTH-1)
        self.log_window.addstr(clear)
        while len(log) > 0:
            line = log[0 : LOG_WINDOW_WIDTH - 1]
            self.log_content.append((logtime,line))
            if len(self.log_content) > (LOG_WINDOW_HEIGHT-1):
                self.log_content.pop(0)
            log = log[LOG_WINDOW_WIDTH - 1:]
        self.log_content = sorted(self.log_content, key=itemgetter(0))
        self.max_log_width = 0
        self.curseslock.acquire()
        for i in range(len(self.log_content)):
            if len(self.log_content[i][1]) > self.max_log_width:
                self.max_log_width = len(self.log_content[i][1])
            j = 0; p = 0
            line_length = len(self.log_content[i][1])
            while j < line_length:
                c = self.log_content[i][1][j]
                if c == '\x1B' and self.log_content[i][1][j:j+2] == '\x1B[': #ESC
                    k = j + 2
                    find_esc_seq = False
                    while k < line_length:
                        if self.log_content[i][1][k] == 'm':
                            find_esc_seq = True
                            break
                        k += 1
                    if find_esc_seq:
                        self.process_esc_sequence(self.log_content[i][1][j:k+1])
                        j = k + 1
                        continue
                if c != '\r' and c != '\n' and c != '\0':
                    self.log_window.addch(i+1, p, c, curses.color_pair(self.cur_color_pair))
                    p += 1
                j += 1
        self.log_window.refresh()
        self.cmd_window.refresh()
        self.curseslock.release()

    def device_list_display(self):
        self.curseslock.acquire()
        self.dev_window.move(1,0)
        clean = (' ' * (DEV_WINDOW_WIDTH-1) + '\n') * (LOG_WINDOW_HEIGHT - 2)
        clean += ' ' * (DEV_WINDOW_WIDTH-1)
        self.dev_window.addstr(clean)
        if len(self.device_list) > 0:
            caddr = self.device_list[0][0:2]
        linenum = 1
        for i in range(len(self.device_list)):
            if self.device_list[i][0:2] != caddr:
                caddr = self.device_list[i][0:2]
                linenum += 1
            if self.device_list[i][0] in self.alias_tuples:
                dev_str = str(i) + '.' + self.alias_tuples[self.device_list[i][0]] + ':' + self.device_list[i][2][5:]
            else:
                dev_str = str(i) + '.' + self.device_list[i][0] + ':' + self.device_list[i][2][5:]
            self.dev_window.addstr(linenum, 0, dev_str)
            linenum += 1
            if linenum >= LOG_WINDOW_HEIGHT:
                break
        self.dev_window.refresh()
        self.cmd_window.refresh()
        self.curseslock.release()

    def cmdrun_command_display(self, cmd):
        self.curseslock.acquire()
        self.cmd_window.move(0, len("Command:"))
        self.cmd_window.addstr(' ' * (LOG_WINDOW_WIDTH + DEV_WINDOW_WIDTH - len("Command:")))
        self.cmd_window.move(0, len("Command:"))
        self.cmd_window.addstr(cmd)
        self.cmd_window.refresh()
        self.curseslock.release()

    def cmdrun_status_display(self, log):
        self.curseslock.acquire()
        self.cmd_window.move(1,0)
        self.cmd_window.addstr(' ' * (LOG_WINDOW_WIDTH + DEV_WINDOW_WIDTH))
        self.cmd_window.move(1,0)
        self.cmd_window.addstr(log)
        self.cmd_window.refresh()
        self.curseslock.release()

    def heartbeat_func(self):
        heartbeat_timeout = time.time() + 10
        while self.keep_running:
            time.sleep(0.05)
            if time.time() >= heartbeat_timeout:
                try:
                    self.service_socket.send(TBframe.construct(TBframe.HEARTBEAT, ''))
                    heartbeat_timeout += 10
                except:
                    continue

    def server_interaction(self):
        msg = ''
        while self.keep_running:
            try:
                new_msg = self.service_socket.recv(MAX_MSG_LENTH)
                if new_msg == '':
                    break

                msg += new_msg
                while msg != '':
                    type, length, value, msg = TBframe.parse(msg)
                    if type == TBframe.TYPE_NONE:
                        break

                    #print type, length
                    if type == TBframe.FILE_BEGIN:
                        if 'file' in locals() and file.closed == False:
                            file.close()
                        filename = 'terminal/' + value
                        file = open(filename, 'w')
                    elif type == TBframe.FILE_DATA:
                        if 'file' in locals() and file.closed == False:
                            file.write(value)
                    elif type == TBframe.FILE_END:
                        if 'file' in locals():
                            file.close()
                    if type == TBframe.ALL_DEV:
                        new_list = []
                        clients = value.split(':')
                        for c in clients:
                            if c == '':
                                continue
                            devs = c.split(',')
                            ip = devs[0]
                            port = devs[1]
                            devs = devs[2:]
                            for d in devs:
                                if d == '':
                                    continue
                                new_list.append([ip, port, d])

                        for dev in new_list:
                            if dev not in self.device_list:
                                self.device_list.append(dev)
                        for dev in list(self.device_list):
                            if dev not in new_list:
                                self.device_list.remove(dev)
                                if dev in self.log_subscribed:
                                    self.log_subscribed.remove(dev)
                        self.device_list.sort()
                        self.device_list_display()
                    if type == TBframe.DEVICE_LOG:
                        dev = value.split(':')[0]
                        logtime = value.split(':')[1]
                        log =value[len(dev) + 1 + len(logtime):]
                        try:
                            logtime = float(logtime)
                        except:
                            continue
                        dev = dev.split(',')
                        if dev not in self.device_list:
                            continue
                        index = self.device_list.index(dev)
                        if dev in self.log_subscribed:
                            log =  str(index) + log
                            self.log_display(logtime, log)
                    if type == TBframe.CMD_DONE:
                        self.cmd_excute_state = 'done'
                    if type == TBframe.CMD_ERROR:
                        self.cmd_excute_state = 'error'
            except:
                break
        self.keep_running = False;

    def parse_device_index(self, index_str):
        try:
            index = int(index_str)
        except:
            return -1
        if index >= len(self.device_list):
            return -1
        return index

    def wait_cmd_excute_done(self, timeout):
        self.cmd_excute_state = 'wait_response'
        while self.cmd_excute_state == 'wait_response':
            time.sleep(0.01)
            timeout -= 0.01
            if timeout <= 0:
                self.cmd_excute_state = "timeout"
                break;

    def send_file_to_server(self, filename):
        if os.path.exists(filename) == False:
            self.cmdrun_status_display("{0} does not exist".format(filename))
            return False
        self.cmdrun_status_display("sending "+ filename + "...")
        file = open(filename,'r')
        content = filename.split('/')[-1]
        data = TBframe.construct(TBframe.FILE_BEGIN, content)
        self.service_socket.send(data)
        content = file.read(1024)
        while(content):
            data = TBframe.construct(TBframe.FILE_DATA, content)
            self.service_socket.send(data)
            content = file.read(1024)
        file.close()
        content = filename
        data = TBframe.construct(TBframe.FILE_END, content)
        self.service_socket.send(data)
        self.cmdrun_status_display("sending "+ filename + "..."+"done")
        return True

    def copy_file_to_client(self, index):
        self.cmdrun_status_display('coping file to {0}...'.format(self.device_list[index][0:2]))
        content = ','.join(self.device_list[index])
        data = TBframe.construct(TBframe.FILE_COPY, content);
        self.service_socket.send(data)
        self.wait_cmd_excute_done(300)
        self.cmdrun_status_display('coping file to {0}...'.format(self.device_list[index][0:2]) + self.cmd_excute_state)
        self.cmd_excute_state = 'idle'

    def erase_devices(self, args):
        devs = args
        if devs == []:
            self.cmdrun_status_display("Usage error: please specify devices you want to programg")
            return False
        succeed = []; failed = []
        for dev in devs:
            index = self.parse_device_index(dev)
            if index == -1:
                self.cmdrun_status_display('invalid device index {0}'.format(dev))
                continue
            status_str = 'erasing {0}.{1}:{2}...'. \
                    format(index, self.device_list[index][0], self.device_list[index][2])
            self.cmdrun_status_display(status_str)
            content = ','.join(self.device_list[index])
            data = TBframe.construct(TBframe.DEVICE_ERASE, content);
            self.service_socket.send(data)
            self.wait_cmd_excute_done(10)
            status_str += self.cmd_excute_state
            self.cmdrun_status_display(status_str)
            if self.cmd_excute_state == "done":
                succeed.append(index)
            else:
                failed.append(index)
            self.cmd_excute_state = 'idle'
        status_str = ''
        if succeed != []:
            status_str += "succeed: {0}".format(succeed)
        if failed != []:
            if status_str != '':
                status_str += ', '
            status_str += "failed: {0}".format(failed)
        self.cmdrun_status_display(status_str)

    def program_devices(self, args):
        if len(args) < 3:
            self.cmdrun_status_display("Usage: programg addresse filename device0 [device1 device2 ... deviceN]")
            return False
        if args[0].startswith('0x') == False:
            self.cmdrun_status_display("Usage error: wrong address input {0}, address should start with 0x".format(args[0]))
            return False
        address  = args[0]
        filename = args[1]
        devs = args[2:]
        file_exist_at = []
        if devs == []:
            self.cmdrun_status_display("Usage error: please specify devices you want to programg")
            return False
        if self.send_file_to_server(filename) == False:
            return False
        succeed = []; failed = []
        for dev in devs:
            index = self.parse_device_index(dev)
            if index == -1:
                self.cmdrun_status_display('invalid device index {0}'.format(dev))
                continue
            if self.device_list[index][0:2] not in file_exist_at:
                if self.copy_file_to_client(index) == False:
                    continue
                file_exist_at.append(self.device_list[index][0:2])
            status_str = 'programming {0} to {1}.{2}:{3}...'. \
                    format(filename, index, self.device_list[index][0], self.device_list[index][2])
            self.cmdrun_status_display(status_str)
            content = ','.join(self.device_list[index]) + ',' + address
            data = TBframe.construct(TBframe.DEVICE_PROGRAM, content);
            self.service_socket.send(data)
            self.wait_cmd_excute_done(90)
            status_str += self.cmd_excute_state
            self.cmdrun_status_display(status_str)
            if self.cmd_excute_state == "done":
                succeed.append(index)
            else:
                failed.append(index)
            self.cmd_excute_state = 'idle'
        status_str = ''
        if succeed != []:
            status_str += "succeed: {0}".format(succeed)
        if failed != []:
            if status_str != '':
                status_str += ', '
            status_str += "failed: {0}".format(failed)
        self.cmdrun_status_display(status_str)

    def reset_devices(self, args):
        if len(args) < 1:
            self.cmdrun_status_display("Usage error, usage: reset devices")
            return False

        for dev in args:
            index = self.parse_device_index(dev)
            if index < 0:
                self.cmdrun_status_display('invalid device index {0}'.format(dev))
                return False

            content = ','.join(self.device_list[index])
            data = TBframe.construct(TBframe.DEVICE_RESET, content)
            self.service_socket.send(data)

    def start_devices(self, args):
        if len(args) < 1:
            self.cmdrun_status_display("Usage error, usage: start devices")
            return False

        for dev in args:
            index = self.parse_device_index(dev)
            if index < 0:
                self.cmdrun_status_display('invalid device index {0}'.format(dev))
                return False

            content = ','.join(self.device_list[index])
            data = TBframe.construct(TBframe.DEVICE_START, content)
            self.service_socket.send(data)

    def stop_devices(self, args):
        if len(args) < 1:
            self.cmdrun_status_display("Usage error, usage: stop devices")
            return False

        for dev in args:
            index = self.parse_device_index(dev)
            if index < 0:
                self.cmdrun_status_display('invalid device index {0}'.format(dev))
                return False

            content = ','.join(self.device_list[index])
            data = TBframe.construct(TBframe.DEVICE_STOP, content)
            self.service_socket.send(data)

    def log_on_off(self, args):
        if len(args) < 2:
            self.cmdrun_status_display("Usage error, usage: log on/off devices")
            return False

        if args[0] == "on":
            type = TBframe.LOG_SUB
        elif args[0] == "off":
            type = TBframe.LOG_UNSUB
        else:
            self.cmdrun_status_display("Usage error, usage: log on/off devices")
            return False

        for dev in args[1:]:
            index = self.parse_device_index(dev)
            if index == -1:
                self.cmdrun_status_display('invalid device index {0}'.format(dev))
                continue
            if (type == TBframe.LOG_SUB) and (self.device_list[index] not in self.log_subscribed):
                data = TBframe.construct(type, ','.join(self.device_list[index]))
                self.service_socket.send(data)
                self.log_subscribed.append(self.device_list[index])
            elif (type == TBframe.LOG_UNSUB) and (self.device_list[index] in self.log_subscribed):
                data = TBframe.construct(type, ','.join(self.device_list[index]))
                self.service_socket.send(data)
                self.log_subscribed.remove(self.device_list[index])

    def log_download(self, args):
        if len(args) < 1:
            self.cmdrun_status_display("Usage error, usage: logdownload device0 [device1 ... deviceN]")
            return False

        for dev in args:
            index = self.parse_device_index(dev)
            if index < 0:
                self.cmdrun_status_display('invalid device index {0}'.format(dev))
                return False

            self.cmdrun_status_display('downloading log file for {0}.{1}:{2}...'. \
                    format(index, self.device_list[index][0], self.device_list[index][2]))
            content = ','.join(self.device_list[index])
            data = TBframe.construct(TBframe.LOG_DOWNLOAD, content)
            self.service_socket.send(data)
            self.wait_cmd_excute_done(300)
            self.cmdrun_status_display('downloading log file for {0}.{1}:{2}...'. \
                    format(index, self.device_list[index][0], self.device_list[index][2]) + self.cmd_excute_state)
            self.cmd_excute_state = 'idle'

    def run_command(self, args, uselast = False):
        if uselast == False:
            if len(args) < 2:
                self.cmdrun_status_display("Usage error, usage: runcmd device cmd_arg0 [cmd_arg1 cmd_arg2 ... cmd_argN]")
                return False

            index = self.parse_device_index(args[0])
            if index < 0:
                self.cmdrun_status_display('invalid device index {0}'.format(args[0]))
                return False
            args = args[1:]
            self.last_runcmd_dev = self.device_list[index]
        else:
            if self.last_runcmd_dev == []:
                self.cmdrun_status_display('Error: you have not excute any remote command with runcmd yet, no target remembered')
                return False

            if len(args) < 1:
                self.cmdrun_status_display("Usage error, usage: !cmd_arg0 [cmd_arg1 cmd_arg2 ... cmd_argN]")
                return False

            if self.last_runcmd_dev not in self.device_list:
                self.cmdrun_status_display("Error: remembered target no longer exists")
                return False
            index = self.device_list.index(self.last_runcmd_dev)

        content = ','.join(self.device_list[index])
        content += ':' + '|'.join(args)
        data = TBframe.construct(TBframe.DEVICE_CMD, content)
        self.service_socket.send(data)

    def client_alias(self, args):
        if len(args) < 1:
            self.cmdrun_status_display("Usage error, usage: alias ip0:name0 [ip1:name1 ... ipN:nameN]")

        for arg in args:
            alias = arg.split(':')
            if len(alias) != 2:
                self.cmdrun_status_display("Usage error, unrecongnized alias tuple {0}".format(arg))
                continue
            self.alias_tuples[alias[0]] = alias[1]
        try:
            file = open("terminal/.alias",'wb')
            pickle.dump(self.alias_tuples, file, protocol=pickle.HIGHEST_PROTOCOL)
            file.close()
        except:
            self.cmdrun_status_display("error: save alias record failed")
        self.device_list_display()

    def print_help_info(self):
        self.log_display(time.time(), "supported commands and usage examples:")
        self.log_display(time.time(), " 1.send       [sd]: send files to sever")
        self.log_display(time.time(), "           example: send a.txt b.txt")
        self.log_display(time.time(), " 2.erase      [er]: erase flash of devices")
        self.log_display(time.time(), "           example: erase 0 1")
        self.log_display(time.time(), " 3.program    [pg]: program fireware to devices")
        self.log_display(time.time(), "           example: program 0x40000 yos_esp32.bin 0 1")
        self.log_display(time.time(), " 4.reset      [rs]: reset/restart devices")
        self.log_display(time.time(), "           example: reset 0 1")
        self.log_display(time.time(), " 5.stop       [sp]: stop devices")
        self.log_display(time.time(), "           example: stop 0 1")
        self.log_display(time.time(), " 6.start      [st]: start devices")
        self.log_display(time.time(), "           example: start 0 1")
        self.log_display(time.time(), " 7.runcmd     [rc]: run command at remote device")
        self.log_display(time.time(), "           example: runcmd 0 ping fc:00:00:10:11:22:33:44")
        self.log_display(time.time(), " 8.^              : run command at latest (runcmd) remote device")
        self.log_display(time.time(), "           example: ^ping fc:00:00:10:11:22:33:44")
        self.log_display(time.time(), " 9.log        [lg]: turn on/off log display for devices, eg.: log on 1")
        self.log_display(time.time(), "           example: log on 1 2; log off 2")
        self.log_display(time.time(), " 10.logdownload[ld]: download log file of device from server")
        self.log_display(time.time(), "           example: logdownload 0 1 2")
        self.log_display(time.time(), " 11.alias     [al]: alias names to ips")
        self.log_display(time.time(), "           example: alias 192.168.1.10:Pi1@HZ")
        self.log_display(time.time(), " 12.help          : print help infomation")

    def process_cmd(self, cmd):
        cmd_argv = cmd.split(' ')
        self.cmdrun_status_display('')
        cmd = cmd_argv[0]; args = cmd_argv[1:]
        if cmd == "send" or cmd == "sd":
            for file in args:
                self.send_file_to_server(file)
        elif cmd == "erase" or cmd == "er":
            self.erase_devices(args)
        elif cmd == "program" or cmd == "pg":
            self.program_devices(args)
        elif cmd == "reset" or cmd == "rs":
            self.reset_devices(args)
        elif cmd == "start" or cmd == "st":
            self.start_devices(args)
        elif cmd == "stop" or cmd == "sp":
            self.stop_devices(args)
        elif cmd == "log" or cmd == "lg":
            self.log_on_off(args)
        elif cmd == "logdownload" or cmd == 'ld':
            self.log_download(args)
        elif cmd == "runcmd" or cmd == "rc":
            self.run_command(args, uselast=False)
        elif cmd[0] == '^':
            self.run_command([cmd[1:]] + args, uselast=True)
        elif cmd == "alias" or cmd == 'al':
            self.client_alias(args)
        elif cmd == "help":
            self.print_help_info()
        else:
            self.cmdrun_status_display("unknown command:" + cmd)

    def user_interaction(self):
        x = len("Command:")
        cmd = ""
        saved_cmd = ""
        history_index = -1
        while self.keep_running:
            c = self.cmd_window.getch()
            if c == ord('\n'):
                if cmd == "q" :
                    self.keep_running = False
                    break
                elif cmd != "":
                    self.process_cmd(cmd)
                    self.cmd_history = [cmd] + self.cmd_history
                cmd = ""
                saved_cmd = ""
                history_index = -1
                self.cmdrun_command_display("")
            elif c == curses.KEY_BACKSPACE: #DELETE
                if cmd == "":
                    continue
                cmd = cmd[0:-1]
                self.cmdrun_command_display(cmd)
            elif c == curses.KEY_UP and len(self.cmd_history) > 0:
                if history_index == -1:
                    saved_cmd = cmd
                if history_index < (len(self.cmd_history) - 1):
                    history_index += 1
                cmd = self.cmd_history[history_index]
                self.cmdrun_command_display(cmd)
            elif c == curses.KEY_DOWN and len(self.cmd_history) > 0:
                if history_index <= -1:
                    history_index = -1
                    continue
                history_index -= 1
                if history_index >= 0:
                    cmd = self.cmd_history[history_index]
                else:
                    cmd = saved_cmd
                self.cmdrun_command_display(cmd)
            elif c == curses.KEY_RESIZE:
                try:
                    self.window_redraw()
                    self.device_list_display()
                    self.log_display(time.time(), '')
                    self.cmdrun_command_display(cmd);
                except:
                    self.keep_running = False
                    break
            else:
                try:
                    cmd = cmd + str(unichr(c))
                except:
                    self.cmdrun_status_display("Error: unsupported unicode character {0}".format(c))
                    continue
                self.cmdrun_command_display(cmd)

    def run(self):
        thread.start_new_thread(self.server_interaction, ())
        thread.start_new_thread(self.heartbeat_func, ())
        while self.keep_running:
            try:
                self.user_interaction()
            except:
                self.keep_running = False
                time.sleep(0.2)

    def deinit(self):
        curses.nocbreak()
        self.cmd_window.keypad(0)
        curses.echo()
        curses.endwin()
        self.service_socket.close()
        try:
            if len(self.cmd_history) > 0:
                file = open("terminal/.cmd_history",'wb')
                pickle.dump(self.cmd_history, file, protocol=pickle.HIGHEST_PROTOCOL)
                file.close()
        except:
            self.cmdrun_status_display("error: save command history failed")

    def terminal_func(self, server_ip, server_port):
        ret = self.init(server_ip, server_port + 1)
        if ret == "success":
            self.run()
        if ret != "connect failed":
            self.deinit()
        if ret == "UI failed":
            print "initilize UI window failed, try increase your terminal window size first"