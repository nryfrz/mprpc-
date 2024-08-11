#pragma once
#include "google/protobuf/service.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include "rpcprovider.h"
#include "mprpcapplication.h"
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

//框架提供的专门发布rpc服务的网络对象类
class RpcProvider {
public:
    //这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service* service);
    //启动rpc服务节点，开始提供rpc远程调用服务
    void Run();
private:
    muduo::net::EventLoop eventLoop_;
    //连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    //读写回调
    void OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);
    //closure回调操作，用于序列化rpc响应和网络发送
    void sendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
private:
    //服务类型信息
    struct ServiceInfo {
        google::protobuf::Service* service; //保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> methodMap;   //保存服务方法
    };
    //存储注册成功的服务对象和服务方法的信息
    std::unordered_map<std::string, ServiceInfo> serviceMap_;
};