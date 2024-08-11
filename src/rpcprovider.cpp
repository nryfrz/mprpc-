#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
/*
serviceName => service描述
                    =>> service* 记录服务对象
                    methoidName => method方法对象

*/
//这里是框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service* service){
    ServiceInfo serviceinfo;
    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor* serviceDesc = service->GetDescriptor();
    //获取服务名字
    std::string serviceName = serviceDesc->name();
    //获取服务对象service的方法数量
    int serviceCnt = serviceDesc->method_count();
    for(int i = 0; i < serviceCnt; i ++ ) {
        //获取服务对象指定下标的方法的描述
        const google::protobuf::MethodDescriptor* methodDescriptor = serviceDesc->method(i);
        std::string mehtodName =  methodDescriptor->name();
        serviceinfo.methodMap.insert({mehtodName, methodDescriptor});
        // std::cout << "serviceName:" << serviceName << " mehtodName:" << mehtodName << std::endl;
        LOG_INFO("serviceName:%s", serviceName.data());
        LOG_INFO("mehtodName:%s", mehtodName.data());
    }
    serviceinfo.service = service;
    serviceMap_.insert({serviceName, serviceinfo});
    
}
//启动rpc服务节点，开始提供rpc远程调用服务
void RpcProvider::Run(){
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    int port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    std::cout << ip << " " << port << std::endl;
    muduo::net::InetAddress address(ip, port);

    //创建
    muduo::net::TcpServer server(&eventLoop_, address, "RpcServer");
    
    //绑定连接回调和消息读写回调
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    //设置线程数
    server.setThreadNum(4);

    //把当前rpc节点要发布的服务都注册到zk上，这样rpc 消费者就可以在zk上发现服务
    //session timeout 30s zkclient 中有一个网络IO线程每1/3timeout时间给zkserver发送心跳消息
    ZkClient zkCli;
    zkCli.start();
    for(auto servicePoint : serviceMap_) {
        std::string service_path = "/" + servicePoint.first;
        zkCli.create(service_path.data(), nullptr, 0);  //永久性节点
        for(auto& methodPoint : servicePoint.second.methodMap) {
            std::string method_path = service_path + "/" + methodPoint.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.data(), port);
            zkCli.create(method_path.data(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);    //非永久性节点
        }
    }
    //启动
    server.start();
    eventLoop_.loop();

}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn) {
    if(!conn->connected()) {
        conn->shutdown();
    }
}
/*
框架内部 RpcProvider和RpcConsumer协商好之间通信的protobuf数据类型
service_name method_name args   定义proto的message类型，进行数据头序列化和反序列化
head_size(4字节) + head_str + args_size 
std::string insert和copy
*/
//如果远程有rpc服务的调用请求， 那么OnMessage就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer, muduo::Timestamp) {
    //网络上接收rpc调用请求的字符流 
    std::string readBuf = buffer->retrieveAllAsString();
    uint32_t headerSize = 0;
    // readBuf.copy(static_cast<char*>(headerSize), 4, 0);
    std::memcpy(&headerSize, readBuf.data(), sizeof(headerSize));
    //根据headerSizse读取数据头的原始字符流,反序列化数据，得到rpc请求的详情信息
    std::string rpcHeaderStr = readBuf.substr(4, headerSize);
    mprpc::RpcHeader rpcHeader;
    std::string serviceName;
    std::string methodName;
    uint32_t argsSize;
    if(rpcHeader.ParseFromString(rpcHeaderStr)) {
        //反序列化成功
        serviceName = rpcHeader.service_name();
        methodName = rpcHeader.method_name();
        argsSize = rpcHeader.args_size();
    }
    else {
        //失败
        // std::cout << "rpc_haeder_str: " << rpcHeaderStr << "parse error!" << std::endl;
        LOG_INFO("rpc_haeder_str: %s parse error!", rpcHeaderStr.data());
        return;
    }
    //获取rpc请求方法参数字符流
    std::string argsStr = readBuf.substr(4 + headerSize, argsSize);
    //调试
    std::cout << "=================" << std::endl;
    std::cout << "headerSize:" << headerSize << std::endl;
    std::cout << "rpcHeaderStr:" << rpcHeaderStr << std::endl;
    std::cout << "serviceName:" << serviceName << std::endl;
    std::cout << "methodName:" << methodName << std::endl;
    std::cout << "argsSize:" << argsSize << std::endl;
    std::cout << "argsStr:" << argsStr << std::endl;
    std::cout << "=================" << std::endl;

    //获取service对象和metod对象
    auto it = serviceMap_.find(serviceName);
    if(it == serviceMap_.end()) {
        std::cout << serviceName << "is not exist" << std::endl;
        return;
    }
    auto mit = it->second.methodMap.find(methodName);
    if(mit == it->second.methodMap.end()) {
        std::cout << serviceName << ":" << methodName << "is not exist" << std::endl;
        return;
    }
    google::protobuf::Service* service = it->second.service;    //获取service对象 new UserService
    const google::protobuf::MethodDescriptor* method = mit->second; //获取method对象 Login
    //生成rpc方法调用的请求request 和响应response
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(argsStr)) {
        std::cout << "request parse error, content:" << argsStr << std::endl;
        return;
    }
    google::protobuf::Message* response = service->GetResponsePrototype(method).New();
    
    //给下面method方法的调用绑定一个Closure回调函数
    google::protobuf::Closure* done = google::protobuf::NewCallback<RpcProvider, const muduo::net::TcpConnectionPtr&, google::protobuf::Message*>(
                                                        this, &RpcProvider::sendRpcResponse, conn, response);

    //在框架上根据远端rpc请求调用当前rpc方法
    //new UserService().Login(controller, requset, response, done)
    service->CallMethod(method, nullptr, request, response, done);
}

//closure回调操作，用于序列化rpc响应和网络发送
void RpcProvider::sendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response) {
    std::string response_str;
    if(response->SerializeToString(&response_str)) {
        conn->send(response_str);
    }
    else {
        std::cout << "serialize response_str error!" << std::endl;
    }
    conn->shutdown();
}