#!/usr/bin/env python
import os
import os.path
import sys

ROOT_DIR = 'data'
DEFAULT_OUTPUT = 'data.h'
ARRAY_BYTES_PER_LINE = 16


def _read_file(filename):
    f = open(filename, 'rb')
    return f.read()


def _grab_files(path):
    pack_files = {}
    if not path.endswith('/'):
        path += '/'
    path_len = len(path)
    for root, subdirs, files in os.walk(path):
        for filename in files:
            if not filename.startswith('.'):
                local_path = os.path.join(root, filename)
                file_path = local_path[path_len:]
                data = _read_file(local_path)
                pack_files[file_path] = data

    return pack_files


def _report(files):
    total_len = 0
    for filename, data in files.items():
        data_len = len(data)
        print('* {:60s} ({:6d} bytes)'.format(filename, data_len))
        total_len += data_len
    print('\nTotal bytes: {}'.format(total_len))


def _normalize_name(filename):
    return filename.replace('/', '_').replace('.', '_').replace('-','_')

def _chunks(l, n=ARRAY_BYTES_PER_LINE):
    for i in xrange(0, len(l), n):
        yield l[i:i+n]


def _write_array(fout, name, data):
    fout.write('PROGMEM const char %s[] = {\n' % name)
    for chunk in _chunks(data):
        fout.write('    ' +
                   ','.join(map(lambda x: "0x%02x" % ord(x), chunk)) +
                   ',\n')
    fout.write('    };\n\n')


def _write_index(fout, files):
    fout.write('const char *spifless_names[]={\n')
    for filename in files:
        fout.write('    "{}",\n'.format(filename))
    fout.write('    NULL};\n\n')

    fout.write('PGM_P spifless_datas[]={\n')
    for filename in map(_normalize_name, files):
        fout.write('    {},\n'.format(filename))
    fout.write('    };\n\n')

    fout.write('const int spifless_lengths[]={\n')
    for filename, data in files.items():
        fout.write('    {:6d}, // {} \n'.format(len(data), filename))
    fout.write('    };\n\n')


def _pack_files(files, output_file):
    fout = open(output_file, 'w')
    fout.write("#ifndef __SPIFLESS_DATA_H\n"
               "#define __SPIFLESS_DATA_H\n")

    for filename, data in files.items():
       _write_array(fout, _normalize_name(filename), data)

    _write_index(fout, files)
    fout.write("#endif\n")
    fout.close()


def pack_dir(path, output):
    files = _grab_files(path)
    _report(files)
    _pack_files(files, output)


def main():
    path = ROOT_DIR
    output = DEFAULT_OUTPUT
    if len(sys.argv) > 1:
        path = sys.argv[1]
    if len(sys.argv) > 2:
        output = sys.argv[2]

    print('Scanning directory: {}\n'.format(path))
    pack_dir(path, output)
    print('\n{} written.\n'.format(output))


if __name__ == '__main__':
    main()
