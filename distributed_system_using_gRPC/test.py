#!/usr/bin/env python3
import os

supernode_ports = [13245, 13246]
filenames = ['EE324.txt',
    'Paper_abstract.txt',
    'Snow_White_and_the_Seven_Dwarfs.txt',
    'Three_little_pigs.txt']

os.chdir('./cmake/build')
cmd = """
time ./client ../../source_keyword_files/Converted_{:s} 127.0.0.1 {:d}
diff Converted_{:s}.decoded ../../answer_value_files/{:s}
rm Converted_{:s}.decoded
"""

for filename in filenames:
    for port in supernode_ports:
        os.system(cmd.format(filename, port, filename, filename, filename))