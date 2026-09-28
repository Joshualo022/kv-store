import socket, struct, threading

def send_command(sock, message):
    b = message.encode()
    sock.sendall(struct.pack(">I", len(b)) + b)

def recv_exact(sock, n):
    data = b""
    while len(data) < n:
        chunk = sock.recv(n - len(data))
        if not chunk:
            raise ConnectionError("server closed")
        data += chunk
    return data

def recv_response(sock):
    length = struct.unpack(">I", recv_exact(sock, 4))[0]
    return recv_exact(sock, length).decode()

def worker(client_id, num_ops, errors):
    try:
        s = socket.socket()
        s.connect(("localhost", 6379))
        for i in range(num_ops):
            send_command(s, f"SET c{client_id}k{i} v{client_id}_{i}")
            recv_response(s)
        for i in range(num_ops):
            send_command(s, f"GET c{client_id}k{i}")
            if recv_response(s) != f"v{client_id}_{i}":
                errors.append((client_id, i))
        s.close()
    except Exception as e:
        errors.append((client_id, str(e)))

errors = []
threads = [threading.Thread(target=worker, args=(c, 500, errors)) for c in range(5)]
for t in threads: t.start()
for t in threads: t.join()
print("errors:", len(errors), errors[:5])