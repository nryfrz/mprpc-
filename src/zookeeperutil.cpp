#include "zookeeperutil.h"
#include "mprpcapplication.h"

//全局的watcher观察器，zkserver给zkclient返回的通知
void global_watcher(zhandle_t *zh, int type, 
        int state, const char *path,void *watcherCtx) {
            if(type == ZOO_SESSION_EVENT) { //回调的消息类型    和会话相关的消息类型
                if(state == ZOO_CONNECTED_STATE) {
                    sem_t* sem = (sem_t*)zoo_get_context(zh);
                    sem_post(sem);
                }
            }
}

ZkClient::ZkClient() {
    zhandle_ = nullptr;
}

ZkClient::~ZkClient() {
    if(zhandle_ != nullptr) {
        zookeeper_close(zhandle_);
    }
}


//连接zookeeperserver
void ZkClient::start() {
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connStr = ip + ":" + port;
    /*
    zookeeper_mt    多线程版本
    zookeeper的API客户端程序提供三个线程
    API调用线程
    网络IO线程
    watcher回调线程
    */
   zhandle_ = zookeeper_init(connStr.data(), global_watcher, 3000, nullptr, nullptr, 0);
   if (zhandle_ == nullptr)
   {
        std::cout << "zookeeper_init error!" <<std::endl;
        exit(EXIT_FAILURE);
   }
   sem_t sem;
   sem_init(&sem, 0, 0);
   zoo_set_context(zhandle_, &sem);
   sem_wait(&sem);
   std::cout << "zookeeper_init success" << std::endl;
}

void ZkClient::create(const char *path, const char *data, int datalen, int state) {
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
	// 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
	flag = zoo_exists(zhandle_, path, 0, nullptr);
	if (ZNONODE == flag) // 表示path的znode节点不存在
	{
		// 创建指定path的znode节点了
		flag = zoo_create(zhandle_, path, data, datalen,
			&ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
		if (flag == ZOK)
		{
			std::cout << "znode create success... path:" << path << std::endl;
		}
		else
		{
			std::cout << "flag:" << flag << std::endl;
			std::cout << "znode create error... path:" << path << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

std::string ZkClient::getData(const char *path) {
    char buffer[64];
	int bufferlen = sizeof(buffer);
	int flag = zoo_get(zhandle_, path, 0, buffer, &bufferlen, nullptr);
	if (flag != ZOK)
	{
		std::cout << "get znode error... path:" << path << std::endl;
		return "";
	}
	else
	{
		return buffer;
	}
}