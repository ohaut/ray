#!/usr/bin/env python
import binascii
import sys
import struct

if len(sys.argv)<4:
    print "usage:", sys.argv[0], "in_file1 patch out_file"
    sys.exit(1)

f1 = open(sys.argv[1], "rb").read()
patch = open(sys.argv[2], "rb").read()

if patch[0:4] != ("BDF" + chr(1)):
    print "Unknown patch format"
    sys.exit(1)

f1_crc32, out_crc32 = struct.unpack(">II", patch[4:12])

patch = patch[12:]
f1_crc32_ = binascii.crc32(f1) & 0xffffffff

if f1_crc32_ != f1_crc32:
    print "Input file CRC32 does not match %08x != %08x" % (f1_crc32_, f1_crc32)
    sys.exit(1)

data = ""

while len(patch) > 0:
    if ord(patch[0]) & 0x80:
        val = struct.unpack(">I", patch[0:4])[0]
        vbin = "{0:032b}".format(val)
        offset = int(vbin[1:22], 2)
        length = int(vbin[22:], 2) + 1
        data += f1[offset:offset+length]
        patch = patch[4:]
    else:
        length = ord(patch[0]) + 1
        data += patch[1:1+length]
        patch = patch[1+length:]

if (binascii.crc32(data) & 0xffffffff) != out_crc32:
    print "Output file CRC32 does not match"
    sys.exit(1)

outf = open(sys.argv[3], "wb")
outf.write(data)
outf.close()
