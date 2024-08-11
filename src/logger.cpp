#include "logger.h"
#include <time.h>
#include <iostream>
Logger& Logger::GetInstance() {
    static Logger logger;
    return logger;
}

Logger::Logger() {
    //启动写日志线程
    std::thread writeLogger([&]() {
        while(true) {
            time_t now = time(nullptr);
            tm* tmnow = localtime(&now);
            char fileName[128];
            sprintf(fileName, "%d-%d-%d.log.txt", tmnow->tm_year + 1900, tmnow->tm_mon + 1, tmnow->tm_mday);
            FILE* pf = fopen(fileName, "a+");
            if(pf == nullptr) {
                std::cout << "open file: " << fileName << "error!" << std::endl;
                exit(EXIT_FAILURE);
            }
            std::string msg = que_.Pop();
            char buf[128] = {0};
            sprintf(buf, "%d-%d-%d =>[%s]", tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec, level_ == INFO ? "INFO" : "ERROR");
            msg.insert(0, buf);
            msg.append("\n");
            fputs(msg.data(), pf);
            fclose(pf);
        }
    });
    writeLogger.detach();
}

void Logger::setLevel(int level) {
    level_ = level;
}
void Logger::log(std::string msg) {
    que_.Push(msg);
}