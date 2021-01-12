#!/usr/bin/env python3
import sys
import random
import string

def randstr(n : int) -> str:
    payload = ''.join([random.choice(string.printable) for _ in range(n)])
    return payload

if len(sys.argv) == 2:
    filename = sys.argv[1]
else:
    filename = 'input.txt'

MAX_CONTENT_SIZE = 0xFFF7
MAX_FILE_SIZE = 1 << 18

with open(f'.tmp/client/{filename}', 'w') as f:
    payload = randstr(random.randint(0, MAX_FILE_SIZE))
    f.write(payload)