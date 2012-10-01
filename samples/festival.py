#!/usr/bin/python
import sys
import struct
import math
import wave
import os.path

# prepare the sound data like this
# this script currently supports only 8bit files with constant sample rate!
# pushd wav; for f in *.wav; do sox $f -c 1 -b 8 -e mu-law ../wav16k8m/$f lowpass 8000 tempo 1.4 rate -v -I 16000; done; popd
tempo = 1.4

# sounds to prepare
table = [
    #('id', 'text') # diphones separated by /
    ('a', 'a'),
    ('b', 'b/e'),
    ('c', 'c/e'),
    ('\x03', 'c~/e'),
    ('d', 'd/e'),
    ('\x05', 'd~/e'),
    ('e', 'e'),
    ('f', 'e/f'),
    ('g', 'g/e'),
    ('h', 'h/a'),
    ('\x01', 'ch'),
    ('i', 'i'),
    ('j', 'j/e'),
    ('k', 'k/a'),
    ('l', 'e/l'),
    ('m', 'e/m'),
    ('n', 'e/n'),
    ('\x07', 'e/n~'),
    ('o', 'o'),
    ('p', 'p/e'),
    ('q', 'k/v/e'),
    ('r', 'e/r'),
    ('\x04', 'e/r~*'),
    ('s', 'e/s'),
    ('\x02', 'e/s~'),
    ('t', 't'),
    ('\x06', 't~'),
    ('u', 'u'),
    ('v', 'v/e'),
    ('w', 'd/v/o/j/i/#/v/e'),
    ('x', 'i/k/s'),
    ('y', 'i/p/s/i/l/o/n'),
    ('z', 'z/e/t'),
    ('\x08', 'z~/e/t~'),
    ('.', 't/e/c~/k/a'),
    ('=', 'r/o/v/n/a/_/s/e'),
    ('-', 'p/o/m/l/c~/k/a'),
    ('+', 'p/l/u/s'),
    ('?', 'o/t/a/z/n~/i:/k'),
    ('!', 'v/i/k/r~*/i/c~/n~/i:/k'),
    ('\x1f', 'm/e/z/e/r/a'),
    ('\xff', 'ch/i/b/a')
    ]

offset = 256*5

if len(sys.argv)<4:
    print "Usage: %s <est file> <wav directory> <output file> [tempo]"
    sys.exit(1)

if len(sys.argv)>4:
    tempo = float(sys.argv[4])

estfile = sys.argv[1]
wavdir = sys.argv[2]
output = sys.argv[3]

def storechar(table, ch, start, delka, data):
    """write one sound to output file including a record in metadata"""
    pos = table.tell()
    table.seek(ch * 5, 0)
    header = struct.pack("BBBBB",
                       start / (256*256), (start % (256*256)) / 256, (start % (256*256)) % 256,
                       delka / 256, delka % 256)
    table.write(header)

    table.seek(start)
    table.write(data)
    
    return start+delka

def sample(width, rate, time):
    """compute sample position based on framerate and time"""
    return math.ceil((width * rate) * time/tempo)

def word(diphdb, text):
    """prepare one sound from diphone entry text (diphones are delimited by /)"""
    text = text.split('/') + ['#']
    samples = []
    lastch = "#"
    for ch in text:
        diphid = "%s-%s" % (lastch, ch)
        #print diphid
        diphrec = diphdb[diphid]
        f = wave.open(os.path.join(wavdir, "%s.wav" % diphrec[0]), "rb")
        samplerate = f.getframerate()
        samplewidth = f.getsampwidth()
        start = sample(samplewidth, samplerate, diphrec[1])
        end = sample(samplewidth, samplerate, diphrec[3])
        f.setpos(start)
        samples += f.readframes(end - start)
        f.close()
        lastch = ch
        if lastch=="_":
            lastch = '#'

    return samples

# read EST file:)
est = file(estfile, "r")
headerdone = False
diphones = {}

for l in est.readlines():
    # skip EST header
    if l.startswith("EST_Header_End"):
        headerdone = True
        continue
    if not headerdone:
        continue
    if len(l.strip())==0:
        continue

    # read diphone data
    (diphone, wavfile, start, middle, end) = l.strip().split()
    (start, middle, end) = (float(start), float(middle), float(end))
    diphones[diphone] = (wavfile, start, middle, end)

est.close()

# set position to first data byte in output file
fout = file(output, "wb")
pos = offset

# prepare all sounds
for w in table:
    print "".join(w[1].split('/'))
    data = "".join(word(diphones, w[1]))
    print "start: %05d\tlen: %d" % (pos, len(data))
    pos = storechar(fout, ord(w[0]), pos, len(data), data)

fout.close()
