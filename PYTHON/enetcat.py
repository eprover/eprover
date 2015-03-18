#!/usr/bin/env python2.7

import socket,struct,sys,time,threading

class MySocket:

    def __init__(self, sock=None):
        self.buf = ""
        if sock is None:
            self.sock = socket.socket()
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        else:
            self.sock = sock

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
        line = raw_input() + "\n"
        _socket.send(line)

def read_data():
    while True:
        sys.stdout.write(_socket.recv())
        sys.stdout.flush()

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print "Usage : ./enetcat host port"
        exit(1)
    _socket.connect(sys.argv[1], int(sys.argv[2]))
    listener = threading.Thread(target=read_data)
    listener.daemon = True
    listener.start()

    send_data()
