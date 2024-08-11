#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"

#include <rpcprovider.h>

//UserService 原来是一个本地服务没提供两个本地方法Login GetFriendLists
class UserService : public fixbug::UserServiceRpc {

public:
    bool Login(std::string name, std::string pwd) {
        std::cout << "doing local service Logon" << std::endl;
        std::cout << "name: " << name << " pwd: " <<  pwd << std::endl;
        return true;
    }
    /*
    重写基类UserServiceRpc的虚函数，下面的方法都是直接调用
    1. caller ==> Login(LoginRequest) ==> muduo ==> callee
    2. callee ==> Login(LoginRequest) ==> 交到下面重写的Login方法上
    */
    virtual void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done) {
                        
        //框架给业务上报参数LoginRequest,获取相应的数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        //本地业务
        bool login_result = Login(name, pwd);

        //写入响应
        fixbug::ResultCode* res = response->mutable_result();
        res->set_errcode(0);
        res->set_errmsg("");
        response->set_success(login_result);

        //执行回调 响应数据对象的序列化和网络发送(框架完成)
        done->Run();
    }

};

int main(int argc, char** argv) {

    //调用框架初始化
    MprpcApplication::Init(argc, argv);

    //把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    //启动一个rpc服务发布节点 Run以后，进程进入阻塞状态，等待远程rpc调用请求
    provider.Run();

    return 0;
}