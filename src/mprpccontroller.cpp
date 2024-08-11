#include "mprpccontroller.h"

MprpcController::MprpcController()
{
    errText_ = "";
    failed_ = false;
}

void MprpcController::Reset()
{
    errText_ = "";
    failed_ = false;
}

bool MprpcController::Failed() const
{
    return failed_;
}

std::string MprpcController::ErrorText() const
{
    return errText_;
}

void MprpcController::SetFailed(const std::string &reason)
{
    failed_ = true;
    errText_ = reason;
}

void MprpcController::StartCancel()
{
}

bool MprpcController::IsCanceled() const
{
    return false;
}

void MprpcController::NotifyOnCancel(google::protobuf::Closure *callback)
{
}
