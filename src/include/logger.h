#pragma once
#include "localqueue.h"
#include <string>

enum Level{
    INFO,
    ERROR,
};

class Logger {
public:
    static Logger& GetInstance();
    void setLevel(int level);
    void log(std::string);

private:

    Logger();
    Logger(const Logger&) = default;
    Logger(Logger&&) = default;
private:
    LocalQueue<std::string> que_;
    int level_;
};

#define LOG_ERROR(format, ...)\
    do\
    {\
        Logger& logger = Logger::GetInstance();\
        logger.setLevel(ERROR);\
        char buf[1024] = {0};\
        snprintf(buf, 1024,format, ##__VA_ARGS__);\
        logger.log(buf);\
    } while(0);

#define LOG_INFO(format, ...)\
    do\
    {\
        Logger& logger = Logger::GetInstance();\
        logger.setLevel(INFO);\
        char buf[1024] = {0};\
        snprintf(buf, 1024,format, ##__VA_ARGS__);\
        logger.log(buf);\
    } while(0);

