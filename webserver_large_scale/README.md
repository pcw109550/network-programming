## Design Report

0. Did not use bufferevent, only used libevent
1. Explanation about thread pool, task queue, and task allocation (10 points)
    - design of thread pool, task queue
        - Thread pool Design: Managed queue of threads. Constructor inits number of tasks and pushes to queue. After that, enqueue method assigns each task with provided runner function.
        -  Task queue: Maintained a single task queue, which supports single producer: multiple consumer model. Queue operation such as push, pop, size are done are threadsafe using mutex.
    - task allocation policy
        - **1 out of 10** threads will be in charge of maintaining connections with redis server. Will be explained at question 2.
        - Single producer: listener thread(main thread using libevent) produces connection fds(`connfd`)s and opens connection with clients. This connections are abstracted to `Server` class, and stored in `task_queue`.
        - Multiple consumer: **9 out of 10** threads are competing to pop frome `task_queue`, gaining connection to client. `thread_task` will run by each 9 threads. Process:
            - Pop server connection stored in `task_queue`.
            - Receive http request from client using method `http_request->recv_http_request`
            - Pop redis connection stored in `client_queue`(see question 2).
            - Parse http request and sent to redis server using method `redis->send_redis_request`.
            - Receive http request by using method `redis->recv_redis_response`.
            - Send result back to client using method `http_response->send_http_response`.
        - Note that every operation is blocking execpt connection part using libevent, so super slow.

2. Explanation about Managing Redis connection, connection allocation (10 points)
    - Redis connection allocation policy
        - By using **1 out of 10** threads, `client_task` function will run, trying to maintain at most **40** connection with redis server. It will abstract the connection with `Client` class and stored in `client_queue`.
        - Therefore making a connection to redis server is controlled by single seperate thread.