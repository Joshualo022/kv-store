import socket
import struct

s = socket.socket()
s.connect(("localhost", 6379))

def send_command(sock, message):
    msg_bytes = message.encode()
    length = len(msg_bytes)
    sock.send(struct.pack(">I", length) + msg_bytes)

def recv_response(sock):
    length_bytes = sock.recv(4)
    length = struct.unpack(">I", length_bytes)[0]
    return sock.recv(length).decode()

send_command(s, "SET name Josh")
print("SET:", recv_response(s))
send_command(s, "SET name Josh")
print("SET:", recv_response(s))

send_command(s, "GET name")
print("GET:", recv_response(s))

send_command(s, "DEL name")
print("DEL:", recv_response(s))

send_command(s, "GET name")
print("GET after DEL:", recv_response(s))

s.close()