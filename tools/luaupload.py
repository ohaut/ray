#!/usr/bin/env python
import os
import os.path
import select
import socket
import sys


TCP_ADDR = sys.argv[1]
TCP_PORT = 2323
FILENAME = sys.argv[2]
TIMEOUT_SECS = 4


class TelnetConnector(object):
    def __init__(self, addr, port, debug=False):
        self._s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._s.connect((addr, port))
        self._debug = debug
        banner = self.recv(128)
        if self._debug:
            print banner

    def send(self, message):
        return self._s.send(message)

    def recv(self, buffer_size):
        ready = select.select([self._s], [], [], TIMEOUT_SECS)
        if ready[0]:
                return self._s.recv(buffer_size)

    def cmd(self, message, expect_response=True):
        self.send(message+"\n")
        if self._debug:
            print "->", message
        if expect_response:
            res = self.recv(128)
            if res is None:
                print "Retrying cmd..."
                self.cmd(message)
            if self._debug:
                print "<-", res
            return res


class Uploader(object):
    def __init__(self, telnet_conn=None, coalesced_lines=5):
        self._telnet = telnet_conn or TelnetConnector(addr=TCP_ADDR,
                                                      port=TCP_PORT)
        self._coalesced_lines = coalesced_lines

    def _split_lines_(self, lines, n):
        for i in range(0, len(lines), n):
            yield lines[i: i+n]

    def _split_lines(self, lines, n):
        """Split lines in groups of n, while yielding comments individually."""
        y_lines = []
        while len(lines) > 0:
            line, lines = lines[0], lines[1:]
            if line.find("--") >= 0:
                if len(y_lines):
                    yield y_lines
                    y_lines = []
                yield [line]
            else:
                y_lines.append(line)
                if len(y_lines) >= n:
                    yield y_lines
                    y_lines = []
        yield y_lines

    def upload_file(self, filename):
        lines = open(filename, 'r').readlines()
        basename = os.path.basename(filename)
        self._telnet.cmd("file.remove('%s');" % basename)
        self._telnet.cmd("file.open('%s','w+');" % basename)
        self._telnet.cmd("w = file.writeline")

        lines = map(lambda s: s.rstrip("\n").rstrip("\r"), lines)
        for chunk_of_lines in self._split_lines(lines, self._coalesced_lines):
            cmd = 'w([[' + "\n".join(chunk_of_lines) + ']]);'
            self._telnet.cmd(cmd)

        self._telnet.cmd("file.close();")
        if basename != 'init.lua':
            self._telnet.cmd("node.compile('%s');" % basename)
            self._telnet.cmd("file.remove('%s');" % basename)

    def upload_dir(self, directory):
        for root, dirs, files in os.walk(directory):
            for filename in files:
                self.upload_file(os.path.join(root, filename))

    def upload(self, path):
        if os.path.isdir(path):
            self.upload_dir(path)
        else:
            self.upload_file(path)

if __name__ == '__main__':
    telnet = TelnetConnector(addr=TCP_ADDR, port=TCP_PORT, debug=True)
    upd = Uploader(telnet)
    upd.upload(FILENAME)
    telnet.cmd("node.restart();", expect_response=False)
