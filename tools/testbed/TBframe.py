CLIENT_DEV = 'CDEV'
ALL_DEV    = 'ADEV'
DEVICE_LOG = 'DLOG'
DEVICE_CMD = 'DCMD'
DEVICE_ERASE = 'DERS'
DEVICE_PROGRAM = 'DPRG'
DEVICE_RESET = 'DRST'
DEVICE_START = 'DSTR'
DEVICE_STOP = 'DSTP'
LOG_SUB    = 'LGSB'
LOG_UNSUB  = 'LGUS'
LOG_DOWNLOAD = 'LGDL'
FILE_BEGIN = 'FBGN'
FILE_DATA  = 'FDTA'
FILE_END   = 'FEND'
FILE_COPY  = 'FCPY'
CMD_DONE   = 'CMDD'
CMD_ERROR  = 'CMDE'
HEARTBEAT  = 'HTBT'
TYPE_NONE  = 'NONE'

def is_valid_type(type):
    if type == CLIENT_DEV:
        return True
    if type == ALL_DEV:
        return True
    if type == DEVICE_LOG:
        return True
    if type == DEVICE_CMD:
        return True
    if type == DEVICE_ERASE:
        return True
    if type == DEVICE_PROGRAM:
        return True
    if type == DEVICE_RESET:
        return True
    if type == DEVICE_START:
        return True
    if type == DEVICE_STOP:
        return True
    if type == LOG_SUB:
        return True
    if type == LOG_UNSUB:
        return True
    if type == LOG_DOWNLOAD:
        return True
    if type == FILE_BEGIN:
        return True
    if type == FILE_DATA:
        return True
    if type == FILE_END:
        return True
    if type == FILE_COPY:
        return True
    if type == CMD_DONE:
        return True
    if type == CMD_ERROR:
        return True
    if type == HEARTBEAT:
        return True
    return False

def construct(type, value):
    if is_valid_type(type) == False:
        return ''
    frame = '[' + type + '{0:04d}'.format(len(value)) + value + ']'
    return frame

def parse(msg):
    sync = False
    type = TYPE_NONE
    length = 0
    value = ''
    while msg != '':
        if len(msg) < 9:
            type = TYPE_NONE
            length = 0
            value = ''
            break;
        for i in range(len(msg)):
            if msg[i] == '[':
                sync = True
                msg = msg[i:]
                break
        if sync == False:
            msg = ''
            break

        type = msg[1:5]
        if is_valid_type(type) == False:
            sync = False;
            print msg[0:9],"Lose sync because of TYPE error"
            msg = msg[1:]
            continue
        try:
            length = int(msg[5:9])
        except:
            sync = False
            print msg[0:9],"Lose sync because of LENGTH error"
            msg = msg[1:]
            continue
        if len(msg) < length + 10:
            type = TYPE_NONE
            length = 0
            value = ''
            break
        if msg[length + 9] != ']':
            sync = False
            print msg[0:9],"Lose sync because of FOOTER error"
            msg = msg[1:]
            continue
        value = msg[9:length+9]
        msg = msg[length+10:]
        break;
    return type, length, value, msg
