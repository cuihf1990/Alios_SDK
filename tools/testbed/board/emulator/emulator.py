import glob, serial, traceback

def list_devices(os):
    return glob.glob('/dev/emulator-*')

def new_device(port):
    try:
        ser = serial.Serial(port, 115200, timeout = 0.02)
    except:
        ser = None
        traceback.print_exc()
        print 'emulator: open {0} error'.format(port)
    return ser

def erase(port):
    return 'fail'

def program(port, address, file):
    return 'fail'

def control(port, operation):
    return 'fail'
