#!/usr/bin/env python

import socket
import struct
from io import BytesIO as bio
import sys

sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
server_address = '/data/srv/state/dqmgui/dev/render-0/socket'

sock.connect(server_address)
message = bio()
message.write(struct.pack('4I', 16, 5, 0, 0))

sock.sendall(message.getvalue())
sock.close()

