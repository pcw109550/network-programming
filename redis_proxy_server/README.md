## Performance measurements
Set 8kB-sized value into a specific key. Measure the time for running 1,000 concurrent GET requests on the key using `ab -c 1000 -n 1000`.

Tested on `eelab6.kaist.ac.kr:Linux eelab6 4.4.0-189-generic #219-Ubuntu SMP Tue Aug 11 12:26:50 UTC 2020 x86_64 x86_64 x86_64 GNU/Linux`. Final results are tested on `durian4.kaist.ac.kr`.

Test command:
```bash
cd test; ./gen_bigfile.sh; cd .. # generates test/8kvalue.txt 
./redis-server --port 12345 &
./bin/webserver_[fork|thread|libevent] 54321 127.0.0.1 12345 &
ab -v 4 -p test/8kvalue.txt http://127.0.0.1:54321/
ab -v 4 -c 1000 -n 1000 http://127.0.0.1:54321/ee324
```

- Part 1
  - Completed requests: 1000
  - Time taken for test: 0.816 ms
  - 99%-ile completion time: 790 ms
```
Document Path:          /ee324
Document Length:        8000 bytes

Concurrency Level:      1000
Time taken for tests:   0.816 seconds
Complete requests:      1000
Failed requests:        0
Total transferred:      8067000 bytes
HTML transferred:       8000000 bytes
Requests per second:    1224.98 [#/sec] (mean)
Time per request:       816.339 [ms] (mean)
Time per request:       0.816 [ms] (mean, across all concurrent requests)
Transfer rate:          9650.32 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0   34   4.6     34      40
Processing:    42  566 271.9    750     753
Waiting:        1  334 234.8    345     737
Total:         42  600 268.7    779     790

Percentage of the requests served within a certain time (ms)
  50%    779
  66%    782
  75%    784
  80%    785
  90%    787
  95%    788
  98%    789
  99%    790
 100%    790 (longest request)
```
- Part 2
  - Completed requests: 1000
  - Time taken for test: 0.729 ms
  - 99%-ile completion time: 713 ms

```
Document Path:          /ee324
Document Length:        8000 bytes

Concurrency Level:      1000
Time taken for tests:   0.729 seconds
Complete requests:      1000
Failed requests:        0
Total transferred:      8067000 bytes
HTML transferred:       8000000 bytes
Requests per second:    1371.04 [#/sec] (mean)
Time per request:       729.373 [ms] (mean)
Time per request:       0.729 [ms] (mean, across all concurrent requests)
Transfer rate:          10800.96 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0   29   5.4     29      41
Processing:    41  491 231.5    665     679
Waiting:        1  288 229.7    199     665
Total:         42  520 227.3    688     713

Percentage of the requests served within a certain time (ms)
  50%    688
  66%    694
  75%    698
  80%    701
  90%    706
  95%    709
  98%    712
  99%    713
 100%    713 (longest request)
```
- Part 3
  - Completed requests: 1000
  - Time taken for test: 0.502 ms
  - 99%-ile completion time: 472 ms
```
Document Path:          /ee324
Document Length:        8000 bytes

Concurrency Level:      1000
Time taken for tests:   0.502 seconds
Complete requests:      1000
Failed requests:        0
Total transferred:      8067000 bytes
HTML transferred:       8000000 bytes
Requests per second:    1990.84 [#/sec] (mean)
Time per request:       502.300 [ms] (mean)
Time per request:       0.502 [ms] (mean, across all concurrent requests)
Transfer rate:          15683.71 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0   25   8.6     25      43
Processing:    17  263 204.8    447     447
Waiting:        0  183 164.1    171     440
Total:         44  288 197.6    460     472

Percentage of the requests served within a certain time (ms)
  50%    460
  66%    464
  75%    466
  80%    468
  90%    470
  95%    471
  98%    472
  99%    472
 100%    472 (longest request)
```

Will briefly compare the performance of part 1 through 3 and explain the results.
   
### Part 1 vs Part 2

Using threads is slightly faster than using processes(0.729ms vs 0.819ms). Threads are somewhat less expensive than processes. Process control (creating and reaping) is twice as expensive as thread control in linux. Therefore using thread is faster comparing with using processes.

### Part 1 vs Part 3

Using libevent is much more faster than using processes(0.502ms vs 0.819ms). The libevent API provides a mechanism to execute a callback function when a specific event occurs on a file descriptor or after a timeout has been reached. Furthermore, libevent also support callbacks due to signals or regular timeouts. It shows better performance because it is implemented based in terms of select(), poll(), epoll(), kqueue(), etc, which are I/O multiplexing. I/O multiplexing does not have process or thread control overhead, so it is faster.



## Description

Supports url encoding(although server needs to perform decoding). Did not use provided code for url encoding.

### Modularization

I did not put intensive comments because I think my functions/macros are well modularized, leading to easily understanding of logic without comments. You can observe total interaction process by observing
  - `webserver_fork`: main function's while loop
  - `webserver_thread`: thread's task function `task`
  - `webserver_libevent`: libevent callback function `task_callback`

Brief overview of structure:

1. parse HTTP request (GET, POST).
    - `server.[cpp|hpp]`:`Server` class open connection with client.
    - `request.[cpp|hpp]`:`Http_request` class reads HTTP request from client and parse headers and body then validate headers.
2. get the corresponding Redis command (i.e., GET, SET) and its key-value arguments.
    - `redis.[cpp|hpp]`:`Redis` class's job is to parse HTTP target/body.   
3. build Redis request following Redis protocol.
    - `client.[cpp|hpp]`:`Client` class opens connection with redis server.
    - `redis.[cpp|hpp]`:`Redis` class will build redis payload and generate redis request.
4. retrieve Redis response from Redis Server.
    - `redis.[cpp|hpp]`:`Redis` will retrive response and decide status.
5. send response to client via HTTP response body
    - `response.[cpp|hpp]`:`Http_response` class will formulate appropriate HTTP response using redis output.

`Http_request`, `Http_response` class only cares about communicating via HTTP. `Redis` class only cares about generating redis payload and parse redis output.

### Design choice of storing data

My server will parse HTTP body, malloc it and store it. After that, process body content and send to redis. My design has no such `streaming` and will consume much memory. I assumed that content body will be less than `1GB`, otherwise memory allocation will fail.

### Maximum HTTP header size

I have assumed that each header line does not exceed `8192KB`. I think this is pretty reasonable design since most of well known web servers have much lesser contraint than this bound. [Evidence](https://www.tutorialspoint.com/What-is-the-maximum-size-of-HTTP-header-values).

### Testing

Wrote bunch of new testcases! See [test.py](test/test.py).

- `func_get_urlencode`, `func_set_urlencode`
  - Generates key value pairs by sampling in `strings.printable`. Although using totally random bytestrings(from `/dev/urandom`) may also work, I just ignored because I was not familar with python encoding.
- `func_invalid_header`, `func_invalid_header_2`, `func_invalid_header_3`
  - RFC 7230: Http request header must have a host header field
  - `content-length`, `content-type` must be necessary or my server will throw `404`.
- `set_empty_value`
  - Check zero length value works fine.
- `robust_set_[2,3,4,5]`
  - Check server can handle big body request. Needs plenty memspace, over `1.5GB`.
  - `robust_set_5` tests when body length larger than `512MB`
- `func_get_nonexist`
  - Ask server to check nonexistent keys. Check server throws `400`.
- Bunch of more testcases not explained.

Valgrind fails in CI/CD(python script never starts), but no memory leak detected while testing locally.