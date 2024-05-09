#include "sign.h"
#include "../include/cpp-httplib/httplib.h"

class Server
{
public:
    Server(int port);

private:
    httplib::Server svr;
    Sign sign;
    static std::string ConstructResponse(const std::string &sign, const std::string &extra, const std::string &token);
};