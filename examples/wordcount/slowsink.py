#!/usr/bin/python

import os, socket, sys, time

host = ''
port = 2007

if len(sys.argv) > 1:
	mps = float(sys.argv[1])
else:
	mps = 1.0
bps = mps * 1000000
BUFSIZE = int(bps/10) # sleep 100ms at full speed

print "Mbytes/s =", mps

if len(sys.argv) > 3:
	host = sys.argv[2]
	port = int(sys.argv[3])
	print "connecting to %s:%d" % (host, port)
else:
	print "listening on port", port

if host:
	client_socket = socket.create_connection((host, port))
	print "connected to", client_socket.getpeername()
else:
	listen_address = ("", port)
	server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	server_socket.bind(listen_address)
	server_socket.listen(5)

	(client_socket, client_address) = server_socket.accept()
	print "got connection from", client_address

start = time.time()
total_size = 0
dot = bps

while True:
	data = client_socket.recv(BUFSIZE)
	if data:
		size = len(data)
		total_size += size
		if total_size >= dot:
			dot += bps
			sys.stdout.write('.')
			sys.stdout.flush()
		time.sleep(size / bps)
	else:
		print "\ndisconnect"
		client_socket.close()
		break

end = time.time()
elapsed = end - start
print "elapsed seconds %.3f" % elapsed
print "total bytes", total_size
print "throughput bytes/s %.2f" % (total_size / elapsed)
print "throughput Mbytes/s %.3f" % (total_size / elapsed / 1000000)

