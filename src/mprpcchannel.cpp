#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include "mprpcapplication.h"
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
/*
headSize + serviceName methodName argsSize + args
*/
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                              google::protobuf::Message *response, google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor *service = method->service();
    std::string serviceName = service->name();
    std::string methodName = method->name();

    // 获取参数序列化字符串长度
    std::string argsStr;
    uint32_t argsSize = 0;
    if (request->SerializeToString(&argsStr))
    {
        argsSize = argsStr.size();
    }
    else
    {
        controller->SetFailed("Serialize request error");
        return;
    }
    // 定义rpc请求头
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(serviceName);
    rpcHeader.set_method_name(methodName);
    rpcHeader.set_args_size(argsSize);

    std::string rpcHeaderStr;
    uint32_t rpcHeaderSize = 0;
    // 请求头序列化
    if (rpcHeader.SerializeToString(&rpcHeaderStr))
    {
        rpcHeaderSize = rpcHeaderStr.size();
    }
    else
    {
        controller->SetFailed("Serialize rpc header str error");
        return;
    }
    // 组织发送的字符串
    std::string sendRpcStr;
    sendRpcStr.insert(0, std::string(reinterpret_cast<char *>(&rpcHeaderSize), 4));
    sendRpcStr += rpcHeaderStr;
    sendRpcStr += argsStr;

    std::cout << "=================" << std::endl;
    std::cout << "headerSize:" << rpcHeaderSize << std::endl;
    std::cout << "rpcHeaderStr:" << rpcHeaderStr << std::endl;
    std::cout << "serviceName:" << serviceName << std::endl;
    std::cout << "methodName:" << methodName << std::endl;
    std::cout << "argsSize:" << argsSize << std::endl;
    std::cout << "argsStr:" << argsStr << std::endl;
    std::cout << "=================" << std::endl;

    // 发送数据
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // int port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    // std::cout << "ip:" << ip << " port: " << port << std::endl;

    //rpc调用方想调用 提供方提供的serviceName的methodName服务，需要查询zk上该服务的host信息
    ZkClient zkCli;
    zkCli.start();
    std::string method_path = "/" + serviceName + "/" + methodName;
    std::string host_data = zkCli.getData(method_path.c_str());
    if(host_data == "") {
        controller->SetFailed(method_path + " is not exist!");
    }
    int idx = host_data.find(":");
    if(idx == -1) {
        controller->SetFailed(method_path + " is invalid!");
    }
    std::cout << "host_data:" << host_data << std::endl;
    std::string ip = host_data.substr(0, idx);
    int port = atoi(host_data.substr(idx + 1, host_data.size() - idx).data());
    std::cout << "ip:" << ip << " port:" << port <<std::endl;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        
        char et[512];
        sprintf(et, "connect error:%d", errno);
        controller->SetFailed(et);
        close(fd);
        exit(EXIT_FAILURE);
    }
    if (send(fd, sendRpcStr.data(), sendRpcStr.size(), 0) == -1)
    {
        char et[512];
        sprintf(et, "send error:%d", errno);
        controller->SetFailed(et);
        close(fd);
    }

    char readBuf[1024];
    memset(readBuf, 0, 1024);
    int readSize = read(fd, readBuf, 1024);

    std::string responseStr(readBuf, readSize);
    if(!response->ParseFromString(responseStr)) {
        controller->SetFailed("parse error:");
    }

    close(fd);
}