import os, sys, time, subprocess
import socket, thread, threading, glob
import TBframe

DEBUG = True

try:
    import serial
except:
    print "error: you haven't install pyserial yet"
    print "intall@ubuntu: sudo apt-get install python-pip"
    print "               sudo pip install pyserial"
    exit(1)

MAX_MSG_LENTH = 2000

class ConnetionLost(Exception):
    pass

class Client:
    def __init__(self):
        self.service_socket = 0
        self.devices = {}
        self.keep_running = True
        self.connected = False

    def send_device_list(self):
        content = ":".join(list(self.devices))
        data = TBframe.construct(TBframe.CLIENT_DEV, content);
        try:
            self.service_socket.send(data)
        except:
            self.connected = False

    def device_logging(self, port):
        log_time = time.time()
        log = ''
        while self.keep_running:
            if self.connected == False:
                time.sleep(0.1)
                continue
            if self.devices[port]['rlock'].acquire(False):
                if self.devices[port]['serial'].isOpen() == False:
                    try:
                        self.devices[port]['serial'].open()
                    except:
                        if os.path.exists(port) == True:
                            print "device_logging, error: unable to open {0}".format(port)
                        self.devices[port]['rlock'].release()
                        time.sleep(0.1)
                        break
                self.devices[port]['rlock'].release()
                newline = False
                try:
                    while self.devices[port]['rlock'].acquire(False) == True:
                        try:
                            c = self.devices[port]['serial'].read(1)
                        except:
                            time.sleep(0.02)
                            c = ''
                            self.devices[port]['serial'].close()
                        finally:
                            self.devices[port]['rlock'].release()
                        if c != '':
                            if log == '':
                                log_time = time.time()
                            log += c
                            if c == '\n':
                                newline = True
                                break
                        else:
                            break
                except:
                    if DEBUG:
                        raise
                    break
                if log != '' and newline == True:
                    log = port + ":{0:.3f}:".format(log_time) + log
                    data = TBframe.construct(TBframe.DEVICE_LOG,log)
                    log = ''
                    try:
                        self.service_socket.send(data)
                    except:
                        self.connected == False
                        continue
            else:
                time.sleep(0.02)
        self.devices[port]['serial'].close()
        self.devices.pop(port)
        print "device {0} removed".format(port)
        self.send_device_list()
        print "device logging thread for {0} exited".format(port)

    def device_monitor(self):
        while self.keep_running:
            devices_new = glob.glob("/dev/espif-*")
            devices_new.sort()
            for port in devices_new:
                if port in list(self.devices):
                    continue
                try:
                    ser = serial.Serial(port, 115200, timeout = 0.02)
                    ser.setDTR(True)
                    ser.setRTS(True)
                except:
                    print "device_monitor, error: unable to open {0}".format(port)
                    continue
                print "device {0} added".format(port)
                self.devices[port] = {'rlock':threading.RLock(), 'wlock':threading.RLock(), 'model':'esp32', 'serial':ser}
                thread.start_new_thread(self.device_logging, (port,))
                self.send_device_list()

            devices_new = glob.glob("/dev/mxchip-*")
            devices_new.sort()
            for port in devices_new:
                if port in list(self.devices):
                    continue
                try:
                    ser = serial.Serial(port, 921600, timeout = 0.02)
                    ser.setRTS(False)
                except:
                    print "device_monitor, error: unable to open {0}".format(port)
                    continue
                print "device {0} added".format(port)
                self.devices[port] = {'rlock':threading.RLock(), 'wlock':threading.RLock(), 'model':'mk3060', 'serial':ser}
                thread.start_new_thread(self.device_logging, (port,))
                self.send_device_list()
            time.sleep(0.05)

    def esp32_erase(self, port):
        if self.devices[port]['serial'].isOpen() == True:
            self.devices[port]['serial'].close()
        retry = 3
        baudrate = 921600
        error = "fail"
        while retry > 0:
            script = ['timeout', '10', 'python']
            script += [os.environ['ESPTOOL_PATH'] + '/esptool.py']
            script += ['--chip']
            script += ['esp32']
            script += ['--port']
            script += [port]
            script += ['--baud']
            script += [str(baudrate)]
            script += ['erase_flash']
            ret = subprocess.call(script)
            if ret == 0:
                error = "success"
                break
            retry -= 1
            baudrate = baudrate / 2
        return error

    def erase_device(self, port):
        ret = "fail"
        if os.path.exists(port) == False:
            return ret

        if self.devices[port]['model'] == "esp32":
            self.devices[port]['rlock'].acquire()
            self.devices[port]['wlock'].acquire()
            ret = esp32_erase(port)
            self.devices[port]['wlock'].release()
            self.devices[port]['rlock'].release()
        return ret

    def esp32_program(self, port, address, file):
        if self.devices[port]['serial'].isOpen() == True:
            self.devices[port]['serial'].close()
        retry = 3
        baudrate = 921600
        error = "fail"
        while retry > 0:
            script = ['timeout', '60', 'python']
            script += [os.environ['ESPTOOL_PATH'] + '/esptool.py']
            script += ['--chip']
            script += ['esp32']
            script += ['--port']
            script += [port]
            script += ['--baud']
            script += [str(baudrate)]
            script += ['--before']
            script += ['default_reset']
            script += ['--after']
            script += ['hard_reset']
            script += ['write_flash']
            script += ['-z']
            script += ['--flash_mode']
            script += ['dio']
            script += ['--flash_freq']
            script += ['40m']
            script += ['--flash_size']
            script += ['4MB']
            script += [address]
            script += [file]
            ret = subprocess.call(script)
            if ret == 0:
                error =  "success"
                break
            retry -= 1
            baudrate = baudrate / 2
        return error

    def mxchip_program(self, port, address, file):
        retry = 3
        baudrate = 921600
        error = "fail"
        while retry > 0:
            script = ['timeout', '80', 'python']
            script += ['autoscripts/yos_firmware_update.py']
            script += [port]
            script += [address]
            script += [file]
            script += ["--hardreset"]
            ret = subprocess.call(script)
            if ret == 0:
                error =  "success"
                break
            retry -= 1
            baudrate = baudrate / 2
            time.sleep(4)
        return error

    def program_device(self, port, address, file):
        ret = "fail"
        if os.path.exists(port) == False:
            return "fail"
        if os.path.exists(file) == False:
            return "fail"

        self.devices[port]['rlock'].acquire()
        self.devices[port]['wlock'].acquire()
        if self.devices[port]['model'] == "esp32":
            ret = self.esp32_program(port, address, file)
        elif self.devices[port]['model'] == "mk3060":
            ret = self.mxchip_program(port, address, file)
        self.devices[port]['wlock'].release()
        self.devices[port]['rlock'].release()
        return ret

    def esp32_control(self, port, operation):
        try:
            if operation == TBframe.DEVICE_RESET:
                print "reset", port
                self.devices[port]['serial'].setDTR(False)
                time.sleep(0.1)
                self.devices[port]['serial'].setDTR(True)
                return "success"
            elif operation == TBframe.DEVICE_STOP:
                print "stop", port
                self.devices[port]['serial'].setDTR(False)
                return "success"
            elif operation == TBframe.DEVICE_START:
                print "start", port
                self.devices[port]['serial'].setDTR(True)
                return "success"
        except:
            pass
        return "fail"

    def mxchip_control(self, port, operation):
        try:
            if operation == TBframe.DEVICE_RESET:
                print "reset", port
                self.devices[port]['serial'].setRTS(True)
                time.sleep(0.1)
                self.devices[port]['serial'].setRTS(False)
                return "success"
            elif operation == TBframe.DEVICE_STOP:
                print "stop", port
                self.devices[port]['serial'].setRTS(True)
                return "success"
            elif operation == TBframe.DEVICE_START:
                print "start", port
                self.devices[port]['serial'].setRTS(False)
                return "success"
        except:
            pass
        return "fail"

    def control_device(self, port, operation):
        if os.path.exists(port) == False:
            return "fail"

        if self.devices[port]['serial'].isOpen() == False:
            try:
                self.devices[port]['serial'].open()
            except:
                if DEBUG:
                    raise
                print "control_device, error: unable to open {0}".format(port)
                return "fail"

        ret = "busy"
        if self.devices[port]['wlock'].acquire(False) == True:
            if self.devices[port]['model'] == "esp32":
                ret = self.esp32_control(port, operation)
            elif self.devices[port]['model'] == "mk3060":
                ret = self.mxchip_control(port, operation)
            else:
                ret = "fail"
            self.devices[port]['wlock'].release()
        else:
            operate= {TBframe.DEVICE_RESET:"reset", TBframe.DEVICE_STOP:"stop", TBframe.DEVICE_START:"start"}
            print operate[operation], port, "failed, device busy"
        return ret

    def heartbeat_func(self):
        heartbeat_timeout = time.time() + 10
        while self.keep_running:
            time.sleep(0.05)
            if self.connected == True and time.time() >= heartbeat_timeout:
                try:
                    self.service_socket.send(TBframe.construct(TBframe.HEARTBEAT, ''))
                    heartbeat_timeout += 10
                except:
                    continue

    def client_func(self, server_ip, server_port):
        self.service_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.service_socket.connect((server_ip, server_port))
            self.connected = True
            print "connect to server {0}:{1} succeeded".format(server_ip, server_port)
        except:
            print "connect to server {0}:{1} failed".format(server_ip, server_port)
            return

        if os.path.exists('client') == False:
            subprocess.call(['mkdir','client'])

        thread.start_new_thread(self.device_monitor,())
        thread.start_new_thread(self.heartbeat_func,())
        msg = ''
        while True:
            try:
                new_msg = self.service_socket.recv(MAX_MSG_LENTH)
                if new_msg == '':
                    raise ConnetionLost
                    break

                msg += new_msg
                while msg != '':
                    type, length, value, msg = TBframe.parse(msg)
                    if type == TBframe.TYPE_NONE:
                        break

                    if type == TBframe.FILE_BEGIN:
                        if 'file' in locals() and file.closed == False:
                            file.close()
                        filename = 'client/' + value
                        file = open(filename, 'w')
                    elif type == TBframe.FILE_DATA:
                        if 'file' in locals() and file.closed == False:
                            file.write(value)
                    elif type == TBframe.FILE_END:
                        if 'file' in locals():
                            file.close()
                    elif type == TBframe.DEVICE_ERASE:
                        args = value.split(',')
                        if len(args) != 3:
                            continue
                        term = args[0:2]
                        port = args[2]
                        result = self.erase_device(port)
                        print "erasing", port, "...", result
                        content = ','.join(term) + ',' + result
                        data = TBframe.construct(TBframe.DEVICE_ERASE, content)
                        try:
                            self.service_socket.send(data)
                        except:
                            raise ConnetionLost
                    elif type == TBframe.DEVICE_PROGRAM:
                        args = value.split(',')
                        if len(args) != 5:
                            continue
                        term = args[0:2]
                        port = args[2]
                        address = args[3]
                        filename = 'client/' + args[4]
                        result = self.program_device(port, address, filename)
                        print "programming", args[4], "to", port, "@", address, "...", result
                        content = ','.join(term) + ',' + result
                        data = TBframe.construct(TBframe.DEVICE_PROGRAM, content)
                        try:
                            self.service_socket.send(data)
                        except:
                            raise ConnetionLost
                    elif type == TBframe.DEVICE_RESET or type == TBframe.DEVICE_START or type == TBframe.DEVICE_STOP:
                        args = value.split(',')
                        if len(args) != 3:
                            continue
                        term = args[0:2]
                        port = args[2]
                        if port not in self.devices:
                            result = "error"
                        else:
                            result = self.control_device(port, type)
                        content = ','.join(term) + ',' + result
                        data = TBframe.construct(type, content)
                        try:
                            self.service_socket.send(data)
                        except:
                            raise ConnetionLost
                    elif type == TBframe.DEVICE_CMD:
                        args = value.split(':')[0]
                        arglen = len(args) + 1
                        args = args.split(',')
                        term = args[0:2]
                        port = args[2]
                        cmds = value[arglen:].split('|')
                        cmds = ' '.join(cmds) +'\r'
                        if port in self.devices:
                            if self.devices[port]['wlock'].acquire(False) == True:
                                self.devices[port]['serial'].write(cmds)
                                self.devices[port]['wlock'].release()
                                result="success"
                                print "device", port, "run command:", cmds[:-1]+", succeed"
                            else:
                                result = "busy"
                                cmds[-1] = ','
                                print "device", port, "run command:", cmds[:-1]+", failed, device busy"
                        else:
                            result = "error"
                        content = ','.join(term) + ',' + result
                        data = TBframe.construct(type, content)
                        try:
                            self.service_socket.send(data)
                        except:
                            raise ConnetionLost
            except ConnetionLost:
                self.connected = False
                print "connection to server lost, try reconnecting..."
                self.service_socket.close()
                self.service_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                while True:
                    try:
                        self.service_socket.connect((server_ip, server_port))
                        self.connected = True
                        self.send_device_list()
                        break
                    except:
                        try:
                            time.sleep(1)
                        except:
                            self.service_socket.close()
                            return
                print "connetion to server resumed"
            except KeyboardInterrupt:
                self.keep_running = False
                time.sleep(0.3)
                break
            except:
                if DEBUG:
                    raise
        self.service_socket.close()
