#include "sign.h"
#include "../include/cpp-httplib/httplib.h"

class Server
{
public:
    Server(int port);

private:
    httplib::Server svr;
    Sign sign;
};