#pragma once
#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"
#include "logger.h"
#include "zookeeperutil.h"
//mprpc框架初始化类
class MprpcApplication {

public:
    static void Init(int argc, char** argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();
private:
    MprpcApplication(){};
    MprpcApplication(const MprpcApplication&) = default;
    MprpcApplication(MprpcApplication&&) = default;
    

private:
    static MprpcConfig config_;
};