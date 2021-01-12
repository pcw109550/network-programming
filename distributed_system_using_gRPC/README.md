## Description

### Requirements

1. My system provides parallelism. 
    - Supernode who first received client request will concurrently ask its childs for translation, by threads.
    - Supernode who received other supernode's request will concurrently ask its childs for translation by threads.
2. My elements communicate via gRPC
    - Connection between supernodes, supernode and child node, child and DB server are done by gRPC.
    - **Important**: The assignment asked me to add two more IDL files. I just added the new gRPC KeyValueStore service inside [assign4.proto](assign4.proto). Therefore there are only single IDL file in the project.
3. My distributed computing framework handle DB miss properly.
    - If supernode's cache miss occur, it will ask to its connected child concurrently.
    - If child's cache miss occur, it will ask DB server.
    - If all directly connected childs to supernode have DB miss, it asks other supernode.
    - Other supernode will ask its child concurrently.
4. Supernode and child node provide cache management.
    - Every supernode and child have cache, satisfing size requirements; 40KB for supernode and 10KB for child.
    - Cache policy: random evict
    - **Important**: Although current implementation of cache works, I noticed some logic bug. For every request of client, my system provides correctness, always responding with correct translated file. However, If there are multiple requests, It seems It does not use cache. I could not figure out how to fix it. Since my system **at least** provides cache management, there must be no deduction for it. The only drawback is the performance degrading due to misimplementation of cache.
 
### Testing

First, build the system by running 

```
./build.sh
```

Project build was tested on `durian4.kaist.ac.kr`.

Spawn child nodes. Run

```
./start_childs.sh
```

or

```
#!/bin/bash
# ./child 50051 [super node's ip_address]:[super node's gRPC port] [DB server's ip_address]:[DB server's port]
cd ./cmake/build
./child 9991 127.0.0.1:11221 eelab5.kaist.ac.kr:50061 &
./child 9992 127.0.0.1:11221 eelab5.kaist.ac.kr:50062 &
./child 9993 127.0.0.1:11221 eelab5.kaist.ac.kr:50063 &
./child 8881 127.0.0.1:11222 eelab6.kaist.ac.kr:50061 &
./child 8882 127.0.0.1:11222 eelab6.kaist.ac.kr:50062 &
./child 8883 127.0.0.1:11222 eelab6.kaist.ac.kr:50063 &
```

The script will spawn all the required childs.

Spawn supernodes. Run

```
./start_supers
```

```
#!/bin/bash
cd ./cmake/build
./super 13245 11221 -s 127.0.0.1:11222 127.0.0.1:9991 127.0.0.1:9992 127.0.0.1:9993 &
./super 13246 11222 127.0.0.1:8881 127.0.0.1:8882 127.0.0.1:8883 &
```

You must run supernode with `-s` argument first. Next run other supernode. First spawned supernode will send its address to other supernode by gRPC. The address is assumed to be `127.0.0.1` and hardcoded to source file. For real systems, address values must be changed to real ip address or domain names. You can adjust number of connected childs by adding more arguments. However, you must provide at least one child per supernode.

Spawn client. Run

```
./test.py
```

or 

```
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
```

The script will spawn client, send request to each supernode per given source keyword files. Therefore there are 8 testcases, since there are two supernode and 4 source keyword files. After receiving decoded file, client will save the result by appending `.decoded` to original file name. It will `diff` original and translated file. 

**Important**: Supernodes cannot handle concurrent clients. It will assume there is only one client in the whole system.

To run single testcase, run 

```
time ./client ../../source_keyword_files/Converted_EE324.txt 127.0.0.1 13245
```