#include "sign.h"
#include "server.h"

#include <iostream>

int main()
{
    try
    {
        sign::InitSignCall();
        server::init(8080);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}