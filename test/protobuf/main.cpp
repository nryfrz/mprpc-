#include "test.pb.h"
#include <iostream>
using namespace fixbug;
int main() {
    // //封装login请求对象的数据
    // LoginRequest re;
    // re.set_name("zhang san");
    // re.set_pwd("123");
    // //序列化
    // std::string sendStr;
    // re.SerializeToString(&sendStr);
    // std::cout << sendStr << std::endl;

    // //反序列化
    // LoginRequest req;
    // req.ParseFromString(sendStr);
    // std::cout << req.name() << std::endl;
    // std::cout << req.pwd() << std::endl;


    // LoginResponse rsp;
    // ResultCode *rc = rsp.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("false");
    // std::string sendStr;
    // rsp.SerializeToString(&sendStr);
    // std::cout << sendStr << std::endl;
    // LoginResponse rs;
    // rs.ParseFromString(sendStr);
    // ResultCode rc1 = rs.result();
    // std::cout << rc1.errcode() << std::endl;

    GetFriendListsResponse rsp;
    ResultCode* res = rsp.mutable_result();
    res->set_errcode(0);

    User* user1 = rsp.add_users();
    user1->set_name("aa");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    User* user2 = rsp.add_users();
    user2->set_name("aa");
    user2->set_age(20);
    user2->set_sex(User::MAN);
    std::cout << rsp.users_size() << std::endl;
    return 0;
}