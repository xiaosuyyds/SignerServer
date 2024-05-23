#include "sign.h"
#include "../include/cpp-httplib/httplib.h"

class Server
{
public:
    Server(int port);

private:
    Sign sign;
    httplib::Server svr;
    std::atomic<uint64_t> counter = 0;

private:
    void Init(int port);
};