#!/usr/bin/python3

import redis
import subprocess
import argparse
import random
import os
import sys
import requests
import random, string
import time
import telnetlib

MAX_VALUESIZE = 1 << 29 # 512 MB


def random_str(length, letter_set=string.ascii_lowercase):
    return ''.join(random.choice(letter_set) for i in range(length))

def is_printable(s):
    return all([c in string.printable for c in s])

class Testset:

    def __init__(self, server_port, redis_ip, redis_port):
        self.server_port = server_port
        self.redis = redis.Redis(host=redis_ip, port=redis_port)

    def func_get(self, key_len=16, val_len=32, printvals=True):
        k = random_str(key_len)
        v = random_str(val_len)
        self.redis.set(k, v)
        if printvals:
            print("Testing key=%r" % k)
        r = requests.get('http://127.0.0.1:{}/{}'.format(self.server_port, k))

        if not r.ok:
            print("Got status code of {}".format(r.status_code))
            return False
        
        if printvals:
            print("Expected: %r, Got: %r" % (v, r.text))
        return v == r.text

    def func_get_urlencode(self, key_len=16, val_len=32, printvals=True):
        # lets just try printables due to encoding problem
        k = random_str(key_len, letter_set=string.printable)
        v = random_str(val_len, letter_set=string.printable)
        self.redis.set(k, v)
        k_enc = requests.utils.quote(k)
        v_enc = requests.utils.quote(v)
        if printvals:
            print("Testing key=%r" % k_enc)
        headers = {"content-type": "application/x-www-form-urlencoded"}
        r = requests.get('http://127.0.0.1:{}/{}'.format(self.server_port, k_enc), headers=headers)
        if not r.ok:
            print("Got status code of {}".format(r.status_code))
            return False
        
        if printvals:
            print("Expected: %r, Got: %r" % (v, r.text))
        return v == r.text

    def func_set(self, cnt=1, key_len=16, val_len=32, printvals=True):
        data = {}
        for i in range(cnt):
            data[random_str(key_len)] = random_str(val_len)

        r = requests.post('http://127.0.0.1:{}/'.format(self.server_port), data=data)

        if not r.ok:
            print("Got status code of {}".format(r.status_code))
            return False

        if r.text != 'OK':
            print("Response body: {}".format(r.text))
            print("Expected: OK")
            return False

        for key, value in data.items():
            stored = self.redis.get(key)
            vb = value.encode('utf-8')
            if printvals:
                print("Expected: %r, Got: %r" % (vb, stored))
            if vb != stored:
                return False
        return True

    def func_set_urlencode(self, cnt=1, key_len=16, val_len=32, printvals=True):
        data = {}
        for i in range(cnt):
            k = random_str(key_len, letter_set=string.printable)
            v = random_str(val_len, letter_set=string.printable)
            data[k] = v

        headers = {"content-type": "application/x-www-form-urlencoded"}
        # requests will encode headers
        r = requests.post('http://127.0.0.1:{}/'.format(self.server_port), data=data, headers=headers)

        if not r.ok:
            print("Got status code of {}".format(r.status_code))
            return False

        if r.text != 'OK':
            print("Response body: {}".format(r.text))
            print("Expected: OK")
            return False

        for key, value in data.items():
            stored = self.redis.get(key)
            vb = value.encode('utf-8')
            if printvals:
                print("Expected: %r, Got: %r" % (vb, stored))
            if vb != stored:
                return False
        return True

    def func_get_nonexist(self, key_len=16, val_len=32):
        k = random_str(key_len)  # Scenario 4: get non-existing key value via GET
        print("Testing non-existing key=%r" % k)
        r = requests.get('http://127.0.0.1:{}/{}'.format(self.server_port, k))

        if r.text != 'ERROR':
            print("Response body: {}".format(r.text))
            print("Expected: ERROR")
            return False

        print("Got status code of {}".format(r.status_code))
        return not r.ok and r.status_code == 404

    def func_invalid_header(self):
        # RFC 7230: Http request header must have a host header field
        # Server must return 400 Bad Request when there is no host value
        print("Testing invalid request header")
        tn = telnetlib.Telnet(host='localhost', port=self.server_port)
        header = b''
        header += b'POST / HTTP/1.0\r\n'
        header += b'\r\n'
        tn.write(header)
        response = tn.read_until(b'\r\n\r\n')
        try:
            status_code = int(response.split(b'\r\n')[0].split()[1])
        except:
            return False
        return status_code == 400 and b'Bad Request' in response

    def func_invalid_header_2(self):
        print("Testing invalid request header")
        tn = telnetlib.Telnet(host='localhost', port=self.server_port)
        header = b''
        header += b'POST / HTTP/1.0\r\n'
        header += 'Host: 127.0.0.1:{}\r\n'.format(self.server_port).encode()
        header += b'\r\n'
        tn.write(header)
        response = tn.read_until(b'\r\n\r\n')
        try:
            status_code = int(response.split(b'\r\n')[0].split()[1])
        except:
            return False
        return status_code == 400 and b'Bad Request' in response

    def func_invalid_header_3(self):
        print("Testing invalid request header")
        tn = telnetlib.Telnet(host='localhost', port=self.server_port)
        header = b''
        header += b'POST / HTTP/1.0\r\n'
        header += 'Host: 127.0.0.1:{}\r\n'.format(self.server_port).encode()
        header += b'content-length: 0\r\n'
        header += b'\r\n'
        tn.write(header)
        response = tn.read_until(b'\r\n\r\n')
        try:
            status_code = int(response.split(b'\r\n')[0].split()[1])
        except:
            return False
        return status_code == 400 and b'Bad Request' in response

    def func_set_big(self):
        with open('test/bigfile5.txt', 'r') as f:
            payload = f.read()
        idx = payload.find('=')
        k, v = payload[:idx], payload[idx + 1:]
        assert is_printable(k) and is_printable(v)
        data = {k : v}
        r = requests.post('http://127.0.0.1:{}/'.format(self.server_port), data=data)

        if not r.ok:
            print("Got status code of {}".format(r.status_code))
            return False

        for key, value in data.items():
            stored = self.redis.get(key)
            vb = value.encode('utf-8')
            if vb != stored:
                return False
        return True

    def func_set_simple(self):   # Scenario 1: single set key value via POST
        return self.func_set(1)

    def func_set_multiple(self): # Scenario 3: mutiple set key value vis POST
        return self.func_set(8)

    def func_set_simple_urlencode(self):   # Scenario 1: single set key value via POST
        return self.func_set_urlencode(1, printvals=False)

    def func_set_multiple_urlencode(self): # Scenario 3: mutiple set key value vis POST
        return self.func_set_urlencode(8, printvals=False)

    def robust_get_1(self):      # Scenario 2: get key value via GET
        return self.func_get(key_len=1024, val_len=40960)

    def robust_get_1_urlencode(self):      # Scenario 2: get key value via GET
        return self.func_get_urlencode(key_len=1024, val_len=40960, printvals=False)

    def robust_set_1(self):
        return self.func_set(cnt=8, key_len=1024, val_len=40960)

    def robust_set_urlencode(self):
        return self.func_set_urlencode(cnt=8, key_len=1024, val_len=40960, printvals=False)

    def robust_set_2(self):
        return self.func_set(cnt=1024, key_len=1024, val_len=40960, printvals=False)

    def robust_set_3(self):
        return self.func_set(cnt=1024, key_len=1024, val_len=81920, printvals=False)

    def robust_set_4(self):
        return self.func_set(cnt=1024, key_len=1024, val_len=81920 * 4, printvals=False)

    def robust_set_5(self): # body size over 512MB, needs much more ram >= 1.5G
        return self.func_set(cnt=1024, key_len=1024, val_len=81920 * 8, printvals=False)

    def func_set_empty_value(self):
        return self.func_set(cnt=1, key_len=16, val_len=0)

    def func_set_empty_key(self): # will not be tested
        return self.func_set(cnt=1, key_len=0, val_len=16)


if __name__ == "__main__":
    # start redis: ./redis-server --port 6379 
    # MUST start redis for testing set!
    parser = argparse.ArgumentParser()
    parser.add_argument("--test", required=True)
    parser.add_argument("--valgrind", action="store_true", default=False)
    parser.add_argument("--redis-port", type=int, default=6379)
    parser.add_argument("--redis-ip", type=str, default='127.0.0.1')
    parser.add_argument("binary")

    args = parser.parse_args()

    server_port = None
    while True:
        server_port = random.randrange(10000, 30000)
        if os.system("netstat -nltd4 | grep :{}".format(server_port)) != 0:
            break

    if not os.path.exists(args.binary):
        print("File {} does not exist.".format(args.binary), file=sys.stderr)
        sys.exit(1)

    print("Starting server...", file=sys.stderr)

    cmdline = [args.binary, str(server_port), args.redis_ip, str(args.redis_port)]
    if args.valgrind:
        cmdline = ["valgrind", "-v", "--leak-check=full", "--track-origins=yes", "--show-leak-kinds=all"] + cmdline
    proc = subprocess.Popen(cmdline, stderr=subprocess.PIPE if args.valgrind else None)

    test = Testset(server_port, args.redis_ip, args.redis_port)
    if not hasattr(test, args.test):
        print("Invalid test {}.".format(args.test), file=sys.stderr)
        sys.exit(1)

    time.sleep(1)
    test_pass = getattr(test, args.test)()

    if test_pass:
        print("Test Passed.", file=sys.stderr)
    else:
        print("Test Failed.", file=sys.stderr)

    print("Terminating server...", file=sys.stderr)
    proc.terminate()
    _, stderr_out = proc.communicate()

    if args.valgrind:
        try:
            stderr_out = stderr_out.decode("utf-8")
        except:
            pass
        print(stderr_out, file=sys.stderr)

        valgrind_ok = True
        for line in stderr_out.split('\n'):
            if 'ERROR SUMMARY: ' in line:
                if '0 errors' not in line:
                    valgrind_ok = False
                    print("Detected memory leak!", file=sys.stderr)
                    break
        sys.exit(0 if valgrind_ok else 1)
    else:
        sys.exit(0 if test_pass else 1)
