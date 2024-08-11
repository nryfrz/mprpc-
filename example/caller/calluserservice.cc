#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"

int main(int argc, char **argv)
{

    MprpcApplication::Init(argc, argv);
    // 演示远程调用发布的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    fixbug::LoginRequest request;
    fixbug::LoginResponse response;
    request.set_name("aa a");
    request.set_pwd("1234");

    MprpcController controller;

    stub.Login(&controller, &request, &response, nullptr); // RpcChannel::callMethod集中起来做所有rpc方法调用的参数序列化和网络发送

    // 一次rpc调用完成响应
    if (controller.Failed())
    {
        // 失败
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (response.result().errcode() == 0)
        {
            std::cout << "rpc login response: " << response.success() << std::endl;
        }
        else
        {
            std::cout << "rpc login response error: " << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}