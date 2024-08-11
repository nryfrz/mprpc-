#pragma once
#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient {

public:
    ZkClient();
    ~ZkClient();
    void start();
    //在zkserver上根据path创建znode节点state=0就是永久性节点
    void create(const char *path, const char *data, int datalen, int state = 0);
    std::string getData(const char* path);

private:
    zhandle_t* zhandle_;

};