import os, sys, time, platform, json
import socket, thread, threading, subprocess
import TBframe

DEBUG = True
LOCALLOG = False

try:
    import serial
except:
    print "error: you haven't install pyserial yet"
    print 'intall@ubuntu: sudo apt-get install python-pip'
    print '               sudo pip install pyserial'
    sys.exit(1)

if platform.system() == 'Windows':
    import serial.tools.list_ports
else:
    import glob

MAX_MSG_LENTH = 2000

class ConnetionLost(Exception):
    pass

class Client:
    def __init__(self):
        self.service_socket = 0
        self.devices = {}
        self.keep_running = True
        self.connected = False
        bytes = os.urandom(4)
        self.poll_str = '\x1b[t'
        for byte in bytes:
            self.poll_str += '{0:02x}'.format(ord(byte))
        self.poll_str += 'm'

    def send_device_list(self):
        content = ':'.join(list(self.devices))
        data = TBframe.construct(TBframe.CLIENT_DEV, content);
        try:
            self.service_socket.send(data)
        except:
            self.connected = False

    def response_filter(self, port, logstr):
        if self.devices[port]['event'].is_set():
            return
        if len(self.devices[port]['filter']) == 0:
            return

        filter = self.devices[port]['filter']
        if filter['lines_num'] == 0:
            if filter['cmd_str'] in logstr:
                self.devices[port]['filter']['lines_num'] += 1
        elif filter['lines_num'] <= filter['lines_exp']:
            log = logstr.replace('\r', '')
            log = log.replace('\n', '')
            if log != '':
                for filterstr in filter['filters']:
                    if filterstr not in log:
                        continue
                    else:
                        log = log.replace(self.poll_str, '')
                        if log != '':
                            self.devices[port]['filter']['response'].append(log)
                            self.devices[port]['filter']['lines_num'] += 1
                        break
            if self.devices[port]['filter']['lines_num'] > filter['lines_exp']:
                self.devices[port]['event'].set()

    def poll_run_command(self, port, command, lines_expect, timeout):
        filter = {}
        filter['cmd_str'] = self.poll_str + command
        filter['lines_exp'] = lines_expect
        filter['lines_num'] = 0
        filter['filters'] = [self.poll_str]
        filter['response'] = []
        self.devices[port]['filter'] = filter
        self.devices[port]['event'].clear()
        ser    = self.devices[port]['serial']
        with self.devices[port]['wlock']:
            ser.write(filter['cmd_str'] + '\r')
        self.devices[port]['event'].wait(timeout)
        response = self.devices[port]['filter']['response']
        self.devices[port]['filter'] = {}
        return response

    def device_status_poll(self, port):
        poll_interval = 60
        time.sleep(2)
        while port in self.devices:
            try:
                if self.devices[port]['serial'].isOpen() == False:
                    time.sleep(poll_interval)
                    continue

                #poll device model
                response = self.poll_run_command(port, 'devname', 1, 0.3)
                if len(response) == 1:
                    self.devices[port]['attributes']['model'] = response[0]

                #poll device version
                response = self.poll_run_command(port, 'version', 1, 0.3)
                if len(response) == 2:
                    for line in response:
                        if 'kernel version :' in line:
                            self.devices[port]['attributes']['kernel_version'] = line.replace('kernel version :AOS-', '')
                        if 'app version :' in line:
                            self.devices[port]['attributes']['app_version'] = line.replace('app version :APP-', '')

                #poll mesh status
                response = self.poll_run_command(port, 'umesh status', 11, 0.3)
                if len(response) == 11:
                    for line in response:
                        if 'state\t' in line:
                            self.devices[port]['attributes']['state'] = line.replace('state\t', '')
                        elif '\tnetid\t' in line:
                            self.devices[port]['attributes']['netid'] = line.replace('\tnetid\t', '')
                        elif '\tmac\t' in line:
                            self.devices[port]['attributes']['macaddr'] = line.replace('\tmac\t', '')
                        elif '\tsid\t' in line:
                            self.devices[port]['attributes']['sid'] = line.replace('\tsid\t', '')
                        elif '\tnetsize\t' in line:
                            self.devices[port]['attributes']['netsize'] = line.replace('\tnetsize\t', '')
                        elif '\trouter\t' in line:
                            self.devices[port]['attributes']['router'] = line.replace('\trouter\t', '')
                        elif '\tchannel\t' in line:
                            self.devices[port]['attributes']['channel'] = line.replace('\tchannel\t', '')

                #poll mesh nbrs
                response = self.poll_run_command(port, 'umesh nbrs', 33, 0.3)
                if len(response) > 0 and 'num=' in response[-1]:
                    self.devices[port]['attributes']['nbrs'] = {}
                    index = 0
                    for line in response:
                        if '\t' not in line or ',' not in line:
                            continue
                        line = line.replace('\t', '')
                        self.devices[port]['attributes']['nbrs']['{0:02d}'.format(index)] = line
                        index += 1

                #poll mesh extnetid
                response = self.poll_run_command(port, 'umesh extnetid', 1, 0.3)
                if len(response) == 1:
                    self.devices[port]['attributes']['extnetid'] = response[0]

                #poll uuid
                response = self.poll_run_command(port, 'uuid', 1, 0.3)
                if len(response) == 1 and 'uuid:' in response[0]:
                    self.devices[port]['attributes']['uuid'] = response[0].replace('uuid: ', '')

                content = port + ':' + json.dumps(self.devices[port]['attributes'], sort_keys=True)
                data = TBframe.construct(TBframe.DEVICE_STATUS, content)
                self.service_socket.send(data)
            except:
                if port not in self.devices:
                    break
                if DEBUG:
                    raise
            time.sleep(poll_interval)
        print 'devie status poll thread for {0} exited'.format(port)

    def device_log_poll(self, port):
        log_time = time.time()
        log = ''
        if LOCALLOG:
            logfile= 'client/' + port.split('/')[-1] + '.log'
            flog = open(logfile, 'a')
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
                            print 'device_serve_thread, error: unable to open {0}'.format(port)
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
                            if DEBUG:
                                raise
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
                    self.response_filter(port, log)
                    log = port + ':{0:.3f}:'.format(log_time) + log
                    if LOCALLOG:
                        flog.write(log)
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
        if LOCALLOG:
            flog.close()
        print 'device {0} removed'.format(port)
        self.send_device_list()
        print 'device log poll thread for {0} exited'.format(port)

    def device_monitor(self):
        while self.keep_running:
            os = platform.system()
            if os == 'Windows':
                coms = serial.tools.list_ports.comports()
                devices_new = []
                for com in coms:
                    devices_new.append(com.device)
            elif os == 'Linux':
                devices_new = glob.glob('/dev/mxchip-*')
                devices_new += glob.glob('/dev/espif-*')
                if devices_new == []:
                    devices_new += glob.glob('/dev/ttyUSB*')
            elif os == 'Darwin':
                devices_new = glob.glob('/dev/tty.usbserial*')
            elif 'CYGWIN' in os:
                devices_new = glob.glob('/dev/ttyS*')
            else:
                print 'unsupported os "{0}"'.format(os)
                break

            devices_new.sort()
            for port in devices_new:
                if port in list(self.devices):
                    continue
                try:
                    if 'mxchip' in port:
                        ser = serial.Serial(port, 921600, timeout = 0.02)
                        ser.setRTS(False)
                    elif 'espif' in port:
                        ser = serial.Serial(port, 115200, timeout = 0.02)
                        ser.setDTR(True)
                        ser.setRTS(True)
                    else:
                        ser = serial.Serial(port, 921600, timeout = 0.02)
                        ser.setRTS(False)
                except:
                    print 'device_monitor, error: unable to open {0}'.format(port)
                    continue
                print 'device {0} added'.format(port)
                self.devices[port] = {'rlock':threading.RLock(), 'wlock':threading.RLock(), 'serial':ser, 'event':threading.Event(), 'attributes':{}, 'filter':{}}
                if 'mxchip' in port:
                    self.devices[port]['attributes']['model'] = 'MK3060'
                if 'espif' in port:
                    self.devices[port]['attributes']['model'] = 'ESP32'
                thread.start_new_thread(self.device_log_poll, (port,))
                thread.start_new_thread(self.device_status_poll, (port,))
                self.send_device_list()
            time.sleep(0.1)
        print 'device monitor thread exited'
        self.keep_running = False

    def esp32_erase(self, port):
        if self.devices[port]['serial'].isOpen() == True:
            self.devices[port]['serial'].close()
        retry = 3
        baudrate = 921600
        error = 'fail'
        while retry > 0:
            script = ['python']
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
                error = 'success'
                break
            retry -= 1
            baudrate = baudrate / 2
        return error

    def device_erase(self, port, term):
        ret = 'fail'
        model = 'unknown'
        if 'model' in self.devices[port]['attributes']:
            model = self.devices[port]['attributes']['model']
        model = model.lower()

        if model == 'esp32':
            self.devices[port]['rlock'].acquire()
            self.devices[port]['wlock'].acquire()
            self.devices[port]['serial'].close()
            ret = esp32_erase(port)
            self.devices[port]['serial'].open()
            self.devices[port]['wlock'].release()
            self.devices[port]['rlock'].release()
            print 'erasing', port, '...', ret
        else:
            print "error: erasing dose not support device '{0}'".format(model)
        content = ','.join(term) + ',' + ret
        self.send_packet(TBframe.DEVICE_ERASE, content)

    def esp32_program(self, port, address, file):
        if self.devices[port]['serial'].isOpen() == True:
            self.devices[port]['serial'].close()
        retry = 3
        baudrate = 921600
        error = 'fail'
        while retry > 0:
            script = ['python']
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
                error =  'success'
                break
            retry -= 1
            baudrate = baudrate / 2
        return error

    def mxchip_program(self, port, address, file):
        retry = 3
        error = 'fail'
        while retry > 0:
            script = ['python', 'aos_firmware_update.py']
            script += [port]
            script += [address]
            script += [file]
            script += ['--hardreset']
            ret = subprocess.call(script)
            if ret == 0:
                error =  'success'
                break
            retry -= 1
            time.sleep(4)
        return error

    def device_program(self, port, address, file, term):
        ret = 'fail'
        model = 'unknown'

        if 'model' in self.devices[port]['attributes']:
            model = self.devices[port]['attributes']['model']
        model = model.lower()

        if model == 'esp32':
            self.devices[port]['rlock'].acquire()
            self.devices[port]['wlock'].acquire()
            self.devices[port]['serial'].close()
            ret = self.esp32_program(port, address, file)
            self.devices[port]['serial'].open()
            self.devices[port]['wlock'].release()
            self.devices[port]['rlock'].release()
            print 'programming', file, 'to', port, '@', address, '...', ret
        elif model == 'mk3060':
            self.devices[port]['rlock'].acquire()
            self.devices[port]['wlock'].acquire()
            self.devices[port]['serial'].close()
            ret = self.mxchip_program(port, address, file)
            self.devices[port]['serial'].open()
            self.devices[port]['wlock'].release()
            self.devices[port]['rlock'].release()
            print 'programming', file, 'to', port, '@', address, '...', ret
        else:
            print "error: programing dose not support device '{0}'".format(model)
        content = ','.join(term) + ',' + ret
        self.send_packet(TBframe.DEVICE_PROGRAM, content)

    def esp32_control(self, port, operation):
        try:
            if operation == TBframe.DEVICE_RESET:
                print 'reset', port
                self.devices[port]['serial'].setDTR(False)
                time.sleep(0.1)
                self.devices[port]['serial'].setDTR(True)
                return 'success'
            elif operation == TBframe.DEVICE_STOP:
                print 'stop', port
                self.devices[port]['serial'].setDTR(False)
                return 'success'
            elif operation == TBframe.DEVICE_START:
                print 'start', port
                self.devices[port]['serial'].setDTR(True)
                return 'success'
        except:
            pass
        return 'fail'

    def mxchip_control(self, port, operation):
        try:
            if operation == TBframe.DEVICE_RESET:
                print 'reset', port
                self.devices[port]['serial'].setRTS(True)
                time.sleep(0.1)
                self.devices[port]['serial'].setRTS(False)
                return 'success'
            elif operation == TBframe.DEVICE_STOP:
                print 'stop', port
                self.devices[port]['serial'].setRTS(True)
                return 'success'
            elif operation == TBframe.DEVICE_START:
                print 'start', port
                self.devices[port]['serial'].setRTS(False)
                return 'success'
        except:
            pass
        return 'fail'

    def device_control(self, port, operation):
        if port not in self.devices:
            return 'error'

        if self.devices[port]['serial'].isOpen() == False:
            try:
                self.devices[port]['serial'].open()
            except:
                if DEBUG:
                    raise
                print 'device_control, error: unable to open {0}'.format(port)
                return 'fail'

        ret = 'busy'
        model = 'unknown'

        if 'model' in self.devices[port]['attributes']:
            model = self.devices[port]['attributes']['model']
        model = model.lower()

        if self.devices[port]['wlock'].acquire(False) == True:
            if model == 'esp32':
                ret = self.esp32_control(port, operation)
            elif model == 'mk3060':
                ret = self.mxchip_control(port, operation)
            else:
                ret = 'fail'
            self.devices[port]['wlock'].release()
        else:
            operate= {TBframe.DEVICE_RESET:'reset', TBframe.DEVICE_STOP:'stop', TBframe.DEVICE_START:'start'}
            print operate[operation], port, 'failed, device busy'
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

    def send_packet(self, type, content):
        data = TBframe.construct(type, content)
        try:
            self.service_socket.send(data)
        except:
            raise ConnetionLost

    def client_func(self, server_ip, server_port):
        self.service_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.service_socket.connect((server_ip, server_port))
            self.connected = True
            print 'connect to server {0}:{1} succeeded'.format(server_ip, server_port)
        except:
            print 'connect to server {0}:{1} failed'.format(server_ip, server_port)
            return

        if os.path.exists('client') == False:
            os.mkdir('client')

        self.send_packet(TBframe.CLIENT_TAG, self.poll_str)
        thread.start_new_thread(self.device_monitor,())
        thread.start_new_thread(self.heartbeat_func,())

        file_received = {}
        file_receiving = {}
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

                    for hash in list(file_receiving):
                        if time.time() > file_receiving[hash]['timeout']:
                            file_receiving[hash]['handle'].close()
                            try:
                                os.remove(file_receiving[hash]['name'])
                            except:
                                pass
                            file_receiving.pop(hash)

                    if type == TBframe.FILE_BEGIN:
                        split_value = value.split(':')
                        terminal = split_value[0]
                        hash = split_value[1]
                        filename = split_value[2]
                        if hash in file_received:
                            if os.path.exists(file_received[hash]) == True:
                                content = terminal + ',' + 'exist'
                                self.send_packet(type, content)
                                continue
                            else:
                                file_received.pop(hash)

                        if hash in file_receiving:
                            content = terminal + ',' + 'busy'
                            self.send_packet(type, content)
                            continue

                        filename = 'client/' + filename
                        filename += '-' + terminal.split(',')[0]
                        filename += '@' + time.strftime('%Y-%m-%d-%H-%M')
                        filehandle = open(filename, 'wb')
                        timeout = time.time() + 5
                        file_receiving[hash] = {'name':filename, 'seq':0, 'handle':filehandle, 'timeout': timeout}
                        content = terminal + ',' + 'ok'
                        self.send_packet(type, content)
                        if DEBUG:
                            print 'start receiving {0} as {1}'.format(split_value[2], filename)
                    elif type == TBframe.FILE_DATA:
                        split_value = value.split(':')
                        terminal = split_value[0]
                        hash = split_value[1]
                        seq  = split_value[2]
                        data = value[(len(terminal) + len(hash) + len(seq) + 3):]
                        seq = int(seq)
                        if hash not in file_receiving:
                            content = terminal + ',' + 'noexist'
                            self.send_packet(type, content)
                            continue
                        if file_receiving[hash]['seq'] != seq and file_receiving[hash]['seq'] != seq + 1:
                            content = terminal + ',' + 'seqerror'
                            self.send_packet(type, content)
                            continue
                        if file_receiving[hash]['seq'] == seq:
                            file_receiving[hash]['handle'].write(data)
                            file_receiving[hash]['seq'] += 1
                            file_receiving[hash]['timeout'] = time.time() + 5
                        content = terminal + ',' + 'ok'
                        self.send_packet(type, content)
                    elif type == TBframe.FILE_END:
                        split_value = value.split(':')
                        terminal = split_value[0]
                        hash = split_value[1]
                        if hash not in file_receiving:
                            content = terminal + ',' + 'noexist'
                            self.send_packet(type, content)
                            continue
                        file_receiving[hash]['handle'].close()
                        localhash = TBframe.hash_of_file(file_receiving[hash]['name'])
                        if localhash != hash:
                            response = 'hasherror'
                        else:
                            response = 'ok'
                            file_received[hash] = file_receiving[hash]['name']
                        if DEBUG:
                            print 'finished receiving {0}, result:{1}'.format(file_receiving[hash]['name'], response)
                        file_receiving.pop(hash)
                        content = terminal + ',' + response
                        self.send_packet(type, content)
                    elif type == TBframe.DEVICE_ERASE:
                        args = value.split(',')
                        if len(args) != 3:
                            continue
                        term = args[0:2]
                        port = args[2]
                        thread.start_new_thread(self.device_erase, (port, term,))
                    elif type == TBframe.DEVICE_PROGRAM:
                        args = value.split(',')
                        if len(args) != 5:
                            continue
                        term = args[0:2]
                        port = args[2]
                        address = args[3]
                        hash = args[4]
                        if hash not in file_received:
                            content = ','.join(term) + ',' + 'error'
                            self.send_packet(type, content)
                            continue
                        filename = file_received[hash]
                        thread.start_new_thread(self.device_program, (port, address, filename, term,))
                    elif type == TBframe.DEVICE_RESET or type == TBframe.DEVICE_START or type == TBframe.DEVICE_STOP:
                        args = value.split(',')
                        if len(args) != 3:
                            continue
                        term = args[0:2]
                        port = args[2]
                        result = self.device_control(port, type)
                        content = ','.join(term) + ',' + result
                        self.send_packet(type, content)
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
                                result='success'
                                print 'device', port, 'run command:', cmds[:-1]+', succeed'
                            else:
                                result = 'busy'
                                print 'device', port, 'run command:', cmds[:-1]+', failed, device busy'
                        else:
                            result = 'error'
                        content = ','.join(term) + ',' + result
                        self.send_packet(type, content)
            except ConnetionLost:
                self.connected = False
                print 'connection to server lost, try reconnecting...'
                self.service_socket.close()
                self.service_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                while True:
                    try:
                        self.service_socket.connect((server_ip, server_port))
                        self.connected = True
                        self.send_device_list()
                        self.send_packet(TBframe.CLIENT_TAG, self.poll_str)
                        break
                    except:
                        try:
                            time.sleep(1)
                        except:
                            self.service_socket.close()
                            return
                print 'connection to server resumed'
            except KeyboardInterrupt:
                self.keep_running = False
                time.sleep(0.3)
                break
            except:
                if DEBUG:
                    raise
        self.service_socket.close()
