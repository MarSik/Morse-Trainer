#!/bin/python
import sys
import struct

f = sys.argv[1]
offset = 256*5

table = open(f, "r+b", 0)

try:

    while True:
        try:
            ch = raw_input("znak> ")
            ch = ord(ch[0])

            table.seek(ch * 5, 0)

            data = table.read(5)
            print repr(data)
            data = struct.unpack("BBBBB", data)
            print "st: %d len: %d" % (data[0]*256*256 + data[1]*256 + data[2], data[3]*256+data[4])

            start = offset + int(raw_input("start> "))
            delka = int(raw_input("delka> "))

            table.seek(ch * 5, 0)
            data = struct.pack("BBBBB",
                               start / (256*256), (start % (256*256)) / 256, (start % (256*256)) % 256,
                               delka / 256, delka % 256)
            table.write(data)

        except ValueError:
            raise
            continue
        
except KeyboardInterrupt:
    table.flush()
    table.close()
