#!/bin/bash
# ./child 50051 [super node's ip_address]:[super node's gRPC port] [DB server's ip_address]:[DB server's port]
cd ./cmake/build
./child 9991 127.0.0.1:11221 eelab5.kaist.ac.kr:50061 &
./child 9992 127.0.0.1:11222 eelab5.kaist.ac.kr:50062 &
./child 9993 127.0.0.1:11223 eelab5.kaist.ac.kr:50063 &
./child 8881 127.0.0.1:11231 eelab6.kaist.ac.kr:50061 &
./child 8882 127.0.0.1:11232 eelab6.kaist.ac.kr:50062 &
./child 8883 127.0.0.1:11233 eelab6.kaist.ac.kr:50063 &