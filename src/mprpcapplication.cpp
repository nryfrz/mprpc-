#include "mprpcapplication.h"
#include <unistd.h>
#include <iostream>
#include <string>
MprpcConfig MprpcApplication::config_;
void ShowArgsHelp() {
    std::cout << "format: command -i <configfile>" <<std::endl;
}
void MprpcApplication::Init(int argc, char** argv){
    if(argc < 2) {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string configfile;
    while((c = getopt(argc, argv, "i:")) != -1) {
        switch (c)
        {
        case 'i':
            configfile = std::string(optarg);
            break;
        case '?':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }
    //开始加载配置文件
    config_.LoadConfigFile(configfile.c_str());
    // std::cout << "rpcserverip: " << config_.Load("rpcserverip") << std::endl;
    // std::cout << "rpcserverport: " << config_.Load("rpcserverport") << std::endl;
    // std::cout << "zookeeperip: " << config_.Load("zookeeperip") << std::endl;
    // std::cout << "zookeeperport: " << config_.Load("zookeeperport") << std::endl;
}
MprpcApplication& MprpcApplication::GetInstance() {
    static MprpcApplication app;
    return app;
}
MprpcConfig& MprpcApplication::GetConfig() {
    return config_;
}