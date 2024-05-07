#include "../include/mongoose/mongoose.h"

class Server
{
public:
    Server(int port);

private:
    struct mg_mgr mgr; // Declare event manager

private:
    void Loop();
};