#!/usr/bin/env python
import binascii
import collections
import struct
import sys

MIN_CHUNK = 5
MAX_CHUNK = 1024

if len(sys.argv) < 4:
    print "usage:", sys.argv[0], "in_file1 in_file2 out_file"
    sys.exit(1)

f1 = open(sys.argv[1], "rb").read()
f2 = open(sys.argv[2], "rb").read()

len_f1 = len(f1)
len_f2 = len(f2)


def find_biggest_chunk(places, needle):
    biggest = places[0]
    biggest_l = MIN_CHUNK

    l_needle = len(needle)
    for place in places:
        eq_bytes = MIN_CHUNK
        for l in range(MIN_CHUNK, min(MAX_CHUNK, l_needle)):
            if needle[l] != f1[place + l]:
                break
            eq_bytes += 1
        if eq_bytes > biggest_l:
            biggest_l = eq_bytes
            biggest = place
            if biggest_l == MAX_CHUNK:
                break

    return biggest_l, biggest


def index_firmware(f1):
    index = collections.defaultdict(list)

    for i in range(0, len(f1)-MIN_CHUNK):
        chunk = f1[i:i+5]
        index[chunk].append(i)

    return index

index = index_firmware(f1)


def _gen_header():
    hdr = "BDF"
    hdr += chr(1)  # version
    f1_crc32 = binascii.crc32(f1) & 0xffffffff
    f2_crc32 = binascii.crc32(f2) & 0xffffffff
    hdr += struct.pack(">II", f1_crc32, f2_crc32)
    return hdr


def _gen_unique(unique):
    if len(unique) == 0:
        return ""
    if len(unique) > 128:
        return _gen_unique(unique[:128]) + _gen_unique(unique[128:])
    return chr(len(unique)-1) + unique


def _gen_copy(offset, c_len):
    bs = "1{0:021b}{1:010b}".format(offset, c_len-1)
    return (chr(int(bs[0:8], 2)) +
            chr(int(bs[8:16], 2)) +
            chr(int(bs[16:24], 2)) +
            chr(int(bs[24:32], 2)))


f = open(sys.argv[3], "wb")
f.write(_gen_header())

unique = ""
while len(f2) > MIN_CHUNK:
    chunkplaces = index[f2[:5]]
    if not chunkplaces:
        unique += f2[0]
        f2 = f2[1:]
        continue
    f.write(_gen_unique(unique))
    unique = ""
    chunk_len, chunk_offset = find_biggest_chunk(chunkplaces, f2)
    f.write(_gen_copy(chunk_offset, chunk_len))
    f2 = f2[chunk_len:]

unique += f2
f.write(_gen_unique(unique))
f.close()
