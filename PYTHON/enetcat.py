#!/usr/bin/env python2.7

import socket,struct,sys,time,threading,signal,readline,os
from e_tptp_expander import TPTPExpander

EXIT_COMMAND = "QUIT"

class MySocket:

    def __init__(self):
        self.buf = ""
        self.sock = socket.socket()

    def send(self, data):
        self.sock.sendall(struct.pack('>I', len(data)+4)+data)

    # From : http://code.activestate.com/recipes/408859-socketrecv-three-ways-to-turn-it-into-recvall/
    # and modified
    def recv(self):
        #data length is packed into 4 bytes
        total_len=0
        total_data=[]
        size=sys.maxint
        body_started = False
        size_data = sock_data = '';
        recv_size=265
        while total_len<size:
            if self.buf != "":
                sock_data = self.buf
                self.buf = ""
            else:
                sock_data = self.sock.recv(recv_size)
            if not sock_data:
                return ''
            if not body_started:
                if len(sock_data)>4:
                    size_data+=sock_data
                    size=struct.unpack('>I', size_data[:4])[0]
                    size -= 4
                    total_data.append(size_data[4:])
                    body_started = True
                else:
                    size_data+=sock_data
            else:
                total_data.append(sock_data)
            total_len=sum([len(i) for i in total_data ])
        data = ''.join(total_data)
        if len(data) > size:
            self.buf = data[size:]
            return data[:size]
        else:
            return data

    def connect(self, host, port):
        config = (host, port)
        self.sock.connect(config)

    def close(self):
        self.sock.close()

_socket = MySocket()

def send_data():
    while True:
        line = raw_input()
        if TPTPExpander.is_include(line):
            lines = TPTPExpander.expand_from_string(line, os.getcwd(), True)
            if lines == None:
                print "Error: One of the include files cannot be read!"
                exit_handler(None, None, 1)
            lines = lines.split("\n")
            for line in lines:
                if line == "" or line[-1] != "\n":
                    line += "\n"
                _socket.send(line)
        else:
            if line == "" or line[-1] != "\n":
                line += "\n"
            if line.strip() != EXIT_COMMAND:
                _socket.send(line)
            else:
                _socket.send(EXIT_COMMAND)
                break

def read_data():
    while True:
        data = _socket.recv()
        if data == "":
            break
        else:
            sys.stdout.write(data)
            sys.stdout.flush()

def exit_handler(signal, frame, exit_code=0):
    _socket.send(EXIT_COMMAND)
    _socket.close()
    print "Bye."
    exit(exit_code)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print "Usage : ./enetcat host port"
        exit(1)
    _socket.connect(sys.argv[1], int(sys.argv[2]))
    listener = threading.Thread(target=read_data)
    listener.daemon = True
    listener.start()
    signal.signal(signal.SIGINT, exit_handler)
    send_data()
    listener.join()
    try:
        _socket.close()
    except socket.error:
        pass
    print "Bye."
    exit(0)
