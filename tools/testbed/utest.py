#!/usr/bin/env python

import sys, os, subprocess

DEFAULT_SERVER_PORT = 34567

def print_usage():
    print "Usage: {0} mode [-s xxx.xxx.xx.xx] [-p xxxxx]\n".format(sys.argv[0])
    print "  example 1: server run at 192.168.1.10@default_port"
    print "    server: {0} server".format(sys.argv[0])
    print "    client: {0} client -s 192.168.1.10".format(sys.argv[0])
    print "    terminal: {0} terminal -s 192.168.1.10\n".format(sys.argv[0])
    print "  example 2: server run at 192.168.1.18@12345"
    print "    server: {0} server -p 12345".format(sys.argv[0])
    print "    terminal: {0} client -s 192.168.1.18 -p 12345".format(sys.argv[0])
    print "    terminal: {0} terminal -s 192.168.1.18 -p 12345\n".format(sys.argv[0])

if len(sys.argv) < 2:
    print_usage()
    exit(1)

server_port = DEFAULT_SERVER_PORT
if sys.argv[1] == "client":
    from client import Client
    if len(sys.argv) < 4 or sys.argv[2] != "-s":
        print_usage()
        exit(1)

    tmpfile = "/tmp/uradar_testbed_client"
    if os.path.exists(tmpfile):
        print "An urader testbed client is already running"
        exit(0)

    server_ip = sys.argv[3]
    if len(sys.argv) >= 6 and sys.argv[4] == '-p':
        try:
            server_port = int(sys.argv[5])
            if server_port<0 or server_port > 65534:
                raise BaseException
        except:
            print "Usage error: invalid server port"
            print_usage()
            exit(1)
    subprocess.call(["touch",tmpfile])
    client = Client()
    client.client_func(server_ip, server_port)
    subprocess.call(["rm",tmpfile])
elif sys.argv[1] == "server":
    from server import Server
    if len(sys.argv) >= 4 and sys.argv[2] == '-p':
        try:
            server_port = int(sys.argv[3])
            if server_port<0 or server_port > 65534:
                raise BaseException
        except:
            print "Usage error: invalid server port"
            print_usage()
            exit(1)

    tmpfile = "/tmp/uradar_testbed_server_{0}".format(server_port)
    if os.path.exists(tmpfile):
        print "An urader testbed server is already running at port", server_port
        exit(0)
    subprocess.call(["touch",tmpfile])
    server = Server()
    server.server_func(server_port)
    subprocess.call(["rm",tmpfile])
elif sys.argv[1] == "terminal":
    from terminal import Terminal
    if len(sys.argv) < 4 or sys.argv[2] != "-s":
        print_usage()
        exit(1)
    server_ip = sys.argv[3]
    if len(sys.argv) >= 6 and sys.argv[4] == '-p':
        try:
            server_port = int(sys.argv[5])
            if server_port<0 or server_port > 65534:
                raise BaseException
        except:
            print "Usage error: invalid server port"
            print_usage()
            exit(1)
    terminal = Terminal()
    terminal.terminal_func(server_ip, server_port)
else:
    print_usage()
    exit(0)