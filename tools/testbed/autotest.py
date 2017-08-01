#!/usr/bin/python

import os, sys, time, socket, pdb
import subprocess, thread, threading, pickle
from operator import itemgetter
import TBframe

MAX_MSG_LENTH = 2000
DEBUG = True

class Autotest:
    def __init__(self):
        self.keep_running = True
        self.device_list= {}
        self.service_socket = 0
        self.cmd_excute_state = 'idle'
        self.subscribed = {}
        self.subscribed_reverse = {}
        self.filter = {}
        self.sync_event = threading.Event()
        self.sync_event.clear()


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

    def get_devname_by_devstr(self, devstr):
        if devstr in list(self.subscribed_reverse):
            return self.subscribed_reverse[devstr]
        return ""

    def response_filter(self, devname, logstr):
        if len(self.filter) == 0:
            return

        if self.filter['devname'] != devname:
            return

        if self.filter['lines_exp'] == 0:
            if self.filter['filterstr'] in logstr:
                self.sync_event.set()
        else:
            if self.filter['lines_num'] == 0:
                if self.filter['filterstr'] in logstr:
                    self.filter['lines_num'] += 1
            elif self.filter['lines_num'] <= self.filter['lines_exp']:
                log = logstr.replace("\r", "")
                log = log.replace("\n", "")
                if log != "":
                    self.filter['response'].append(log)
                    self.filter['lines_num'] += 1
                if self.filter['lines_num'] > self.filter['lines_exp']:
                    self.sync_event.set()

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
                    if type == TBframe.ALL_DEV:
                        new_list = {}
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
                                [dev, using] = d.split('|')
                                new_list[ip+','+port+','+dev] = using

                        for dev in list(new_list):
                            self.device_list[dev] = new_list[dev]

                        for dev in list(self.device_list):
                            if dev not in list(new_list):
                                self.device_list.pop(dev)
                                if dev in self.log_subscribed:
                                    self.log_subscribed.remove(dev)
                    if type == TBframe.DEVICE_LOG:
                        dev = value.split(':')[0]
                        logtime = value.split(':')[1]
                        log =value[len(dev) + 1 + len(logtime) + 1:]
                        try:
                            logtime = float(logtime)
                            logtime = time.strftime("%Y-%m-%d %H-%M-%S", time.localtime(logtime));
                        except:
                            continue
                        if dev not in list(self.device_list):
                            continue
                        devname = self.get_devname_by_devstr(dev)
                        if devname != "":
                            self.response_filter(devname, log)
                            log =  devname + ":" + logtime + ":" + log
                            self.logfile.write(log)
                    if type == TBframe.CMD_DONE:
                        self.cmd_excute_state = 'done'
                    if type == TBframe.CMD_ERROR:
                        self.cmd_excute_state = 'error'
            except:
                if DEBUG:
                    raise
                break
        self.keep_running = False;

    def wait_cmd_excute_done(self, timeout):
        self.cmd_excute_state = 'wait_response'
        while self.cmd_excute_state == 'wait_response':
            time.sleep(0.01)
            timeout -= 0.01
            if timeout <= 0:
                self.cmd_excute_state = "timeout"
                break;

    def get_devstr_by_partialstr(self, partialstr):
        devices = list(self.device_list)
        devices.sort()
        for devstr in devices:
            if partialstr in devstr:
                return devstr
        return ""

    def copy_file_to_client(self, devame, filename):
        #argument check
        if devname not in list(self.subscribed):
            print "{0} is not subscribed".format(devname)
            return False
        if os.path.exists(filename) == False:
            print "{0} does not exist".format(filename)
            return False

        #send file to server first
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

        #command server to copy file to client
        content = self.subscribed[device]
        data = TBframe.construct(TBframe.FILE_COPY, content);
        self.service_socket.send(data)
        self.wait_cmd_excute_done(300)
        if self.cmd_excute_state == "done":
            ret = True
        else:
            ret = False
        self.cmd_excute_state = 'idle'
        return ret;

    def device_subscribe(self, devices):
        for devname in list(devices):
            devstr = self.get_devstr_by_partialstr(devices[devname])
            if devstr == "":
                self.subscribed = {}
                self.subscribed_reverse = {}
                return False
            else:
                self.subscribed[devname] = devstr;
                self.subscribed_reverse[devstr] = devname
        for devname in devices:
            data = TBframe.construct(TBframe.LOG_SUB, self.subscribed[devname])
            self.service_socket.send(data)
        return True

    def device_erase(self, devname):
        content = self.subscribed[devname]
        data = TBframe.construct(TBframe.DEVICE_ERASE, content);
        self.service_socket.send(data)
        self.wait_cmd_excute_done(10)
        if self.cmd_excute_state == "done":
            ret = True
        else:
            ret = False
        self.cmd_excute_state = 'idle'
        return ret

    def device_program(self, devname, filename, address):
        if self.copy_file_to_client(devname, filename)== False:
            return False

        content = self.subscribed[devname] + ',' + address
        data = TBframe.construct(TBframe.DEVICE_PROGRAM, content);
        self.service_socket.send(data)
        self.wait_cmd_excute_done(90)
        if self.cmd_excute_state == "done":
            ret = True
        else:
            ret = False
        self.cmd_excute_state = 'idle'
        return ret

    def device_control(self, devname, operation):
        operations = {"start":TBframe.DEVICE_START, "stop":TBframe.DEVICE_STOP, "reset":TBframe.DEVICE_RESET}

        if devname not in list(self.device_list):
            return False
        if operation not in list(operations):
            return False

        content = self.subscribed[devname]
        data = TBframe.construct(operations[operation], content)
        self.service_socket.send(data)
        return True

    def device_run_cmd(self, devname, args, expect_lines = 0, timeout=0.2):
        if devname not in list(self.subscribed):
            return False
        if len(args) == 0:
            return False
        content = self.subscribed[devname]
        content += ':' + '|'.join(args)
        data = TBframe.construct(TBframe.DEVICE_CMD, content)
        self.filter['devname'] = devname
        self.filter['filterstr'] = ' '.join(args)
        self.filter['lines_exp'] = expect_lines
        self.filter['lines_num'] = 0
        self.filter['response'] = []
        self.service_socket.send(data)
        self.sync_event.clear()
        self.sync_event.wait(timeout)
        response = self.filter['response']
        self.filter = {}
        return response

    def start(self, server_ip, server_port, logname):
        #connect to server
        self.service_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.service_socket.connect((server_ip, server_port))
        except:
            print "connect to server {0}:{1} failed".format(server_ip, server_port)
            return False
        if os.path.exists('autotest') == False:
            subprocess.call(['mkdir','autotest'])

        try:
            self.logfile = open("autotest/"+logname, 'w');
        except:
            print "open logfile {0} failed".format(logfile)
            return False

        thread.start_new_thread(self.server_interaction, ())
        thread.start_new_thread(self.heartbeat_func, ())
        time.sleep(0.1)
        return True

    def stop(self):
        self.keep_running = False
        time.sleep(0.2)
        self.service_socket.close()
