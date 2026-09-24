import socket
import struct
import time

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

NUM_OPS = 10000

start = time.time()

for i in range(NUM_OPS):
    send_command(s, f"SET key{i} value{i}")
    recv_response(s)

end = time.time()

elapsed = end - start
print(f"{NUM_OPS} SET ops in {elapsed:.2f}s")
print(f"{NUM_OPS / elapsed:.0f} ops/sec")

s.close()