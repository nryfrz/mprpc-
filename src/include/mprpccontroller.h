#pragma once
#include <google/protobuf/service.h>
#include <string>

class MprpcController : public google::protobuf::RpcController
{
public:
    MprpcController();
    void Reset();

    bool Failed() const;

    std::string ErrorText() const;

    void SetFailed(const std::string &reason);

    //未具体实现
    void StartCancel();

    bool IsCanceled() const;

    void NotifyOnCancel(google::protobuf::Closure *callback);
private:
    bool failed_;
    std::string errText_;
};
