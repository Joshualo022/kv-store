import socket

s = socket.socket()
s.connect(("localhost", 6379))


# GET
s.send(b"GET name")
print("GET:", s.recv(1024).decode())


s.close()