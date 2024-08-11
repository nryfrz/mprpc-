#include "mprpcconfig.h"
#include <iostream>
#include <string>

// 负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *configfile)
{
    FILE *pf = fopen(configfile, "r");
    if (pf == nullptr)
    {
        std::cout << configfile << " is not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (!feof(pf))
    {
        char buf[512];
        fgets(buf, 512, pf);
        // 去掉字符串前面多余的空格
        std::string src_buf(buf);
        Trim(src_buf);
        // 判断#的注释
        if (src_buf[0] == '#' || src_buf.empty())
        {
            continue;
        }
        // 解析配置项
        int idx = src_buf.find('=');
        if (idx == -1)
        {
            continue;
        }
        std::string key(src_buf.substr(0, idx));
        Trim(key);
        //xxxxx=127.0.0.1\n
        int endidx = src_buf.find('\n', idx);

        std::string value(src_buf.substr(idx + 1, endidx - idx - 1));
        Trim(value);
        configMap_.insert({key, value});
    }
}
// 查询配置项信息
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = configMap_.find(key);
    if (it == configMap_.end())
    {
        return std::string();
    }
    return it->second;
}

void MprpcConfig::Trim(std::string &src_buf)
{
    int idx = src_buf.find_first_not_of(' ');
    if (idx != -1)
    {
        // 有空格
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }
    // 去掉，末尾多余空格
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        src_buf = src_buf.substr(0, idx + 1);
    }
}