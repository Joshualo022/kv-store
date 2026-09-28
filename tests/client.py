import socket
import struct
import uuid

HOST, PORT = "localhost", 6379


def connect():
    sock = socket.socket()
    sock.connect((HOST, PORT))
    return sock


def send_command(sock, message):
    data = message.encode()
    sock.sendall(struct.pack(">I", len(data)) + data)


def recv_exact(sock, n):
    data = b""
    while len(data) < n:
        chunk = sock.recv(n - len(data))
        if not chunk:
            raise ConnectionError("server closed the connection")
        data += chunk
    return data


def recv_response(sock):
    length = struct.unpack(">I", recv_exact(sock, 4))[0]
    return recv_exact(sock, length).decode()


def command(sock, message):
    send_command(sock, message)
    return recv_response(sock)


def unique_key(name):
    # The log persists across runs, so every test run needs fresh keys
    return f"{name}_{uuid.uuid4().hex[:8]}"