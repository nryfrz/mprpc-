#pragma once
#include<google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>

class MprpcChannel : public google::protobuf::RpcChannel {
public:
    //所有通过stub调用rpc方法，最终都是调用这会函数，进行统一的数据序列化和网络发送
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done);

};