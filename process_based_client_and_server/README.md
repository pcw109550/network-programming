## Description

Testing environment: `Linux eelab6 4.4.0-189-generic #219-Ubuntu SMP Tue Aug 11 12:26:50 UTC 2020 x86_64 x86_64 x86_64 GNU/Linux` || `Linux ubuntu 5.4.0-47-generic #51~18.04.1-Ubuntu SMP Sat Sep 5 14:35:50 UTC 2020 x86_64 x86_64 x86_64 GNU/Linux` ||  `Ubuntu 18.04.5 LTS x86_64`

I did not put intensive comments because I think my functions/macros are well modularized, leading to easily understanding of logic without comments. The most nontrivial part of my code is the big while loops in main function in both client and server. I thoroughly put comments indicating each handshake steps: `CLIENT_HELLO`, `SERVER_HELLO`, `DATA_DELIVERY`, `DATA_STORE`.

I implemented packet handler functions in [common.c](src/common.c). Both server and client needs packet send and recv methods.

### Design choice of storing files

I first calculate size of file. Maximum content length of packet: `MAX_CONTENT_SIZE = 0xFFF7 = 0xFFFF - 8`. By knowing `MAX_CONTENT_SIZE`, I know how many packets are needed to transfer whole file. Iterate, read bytes from file, formulate packet and send it to server. Server will receive packet, accumulate contents using singly linked list, then store into a file named filename. 

I maintained a singly linked list to store chunks of data. I needed to wait until client sends `DATA_STORE` with filename.

### Methodology of generating sequence number

Used `/dev/urandom` to read out random seed of `srand`. Much more secure than time based seed.

### Assigning sequence number to packets

I followed the below assignment. Let initial seqnum be `x`.

1. `CLIENT_HELLO`  : seqnum `x`
2. `SERVER_HELLO  `: seqnum `x + 1`
3. `DATA_DELIVEREY`: seqnum `x + 1`
4. `DATA_DELIVEREY`: seqnum `x + 2`
5. `DATA_DELIVEREY`: seqnum `x + 3`
6. Some bunch of `DATA_DELIVEREY` packets by incrementing previous seqnum by 1.
7. `DATA_STORE`    : seqnum `x + m`

### Testing

Captured packets and analyze by wireshark. [single.pcap](single.pcap), [multi.pcap](multi.pcap).

```
sudo tcpdump -i lo -w [single.pcap|multi.pcap]
```

Checked out that the packets match the requirements. Used `lo` interface because I only tested with localhost.

I generated random files by using python script: [genfile.py](genfile.py) or bash one liner: [test/simple.sh](test/simple.sh).

Maximum content length of packet: `MAX_CONTENT_SIZE = 0xFFF7 = 0xFFFF - 8`. I generated test cases:
1. Empty file(must generate empty file on serverside)
2. content size `n * MAX_CONTENT_SIZE`, where `n` is positive integer
3. content size `n * MAX_CONTENT_SIZE + m`, where `n` is positive integer and `m` in positive integer less than `MAX_CONTENT_SIZE`.

Maximum filename length is 255 < `MAX_CONTENT_SIZE`, so I always send single `DATA_STORE` to send filename.