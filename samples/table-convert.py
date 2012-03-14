#!/bin/python
import sys
import struct

f = sys.argv[1]
samplerate_in = int(sys.argv[2])
fout = sys.argv[3]
samplerate_out = int(sys.argv[4])

ratio = float(samplerate_out) / samplerate_in

offset = 256*5

table = open(f, "rb", 0)
table_out = open(fout, "wb", 0)

for ch in range(256):
    try:
        table.seek(ch * 5, 0)
        
        data = table.read(5)
        print repr(data)
        data = struct.unpack("BBBBB", data)
        
        oldstart = (data[0]*256*256 + data[1]*256 + data[2]) - offset
        olddelka = (data[3]*256+data[4])

        if oldstart<0:
            oldstart = 0
        
        start = int(round(offset + oldstart*ratio))
        delka = int(round(olddelka * ratio))

        print "%02x '%c' %06d<%05d> -> %06d<%05d>" % (ch, chr(ch), oldstart, olddelka, start - offset, delka)
        
        table.seek(ch * 5, 0)
        data = struct.pack("BBBBB",
                           start / (256*256), (start % (256*256)) / 256, (start % (256*256)) % 256,
                           delka / 256, delka % 256)
        table_out.write(data)

    except ValueError:
        continue
        
table.close()
table_out.flush()
table_out.close()
