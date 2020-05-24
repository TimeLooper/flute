# Flute
阅读和学习以下开源项目源码的学习笔记
+ [muduo](https://github.com/chenshuo/muduo)
+ [ThreadPool](https://github.com/progschj/ThreadPool)
+ [boost](https://github.com/boostorg/boost)
+ [libevent](https://github.com/libevent/libevent)

关于多路IO复用，muduo默认有epoll和poll，另外实现了select和kqueue，其中select在windows和unix下FD_SET结构有所不同，重新实现了FD_SET来突破windows下FD_SET最多64和unix下最多1024个连接的限制，当然即便是这样效率依旧不高