#include "server.h"

Server *server = nullptr;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        try
        {
            std::string ip = "0.0.0.0";
            int port = 8080;
            printf("Start Init server\n");
            server = new Server();
            server->Init();

            printf("Start Init sign\n");
            std::thread([=] { // Cannot use & capture!!!!! will cause crash
                for (int i = 0; i < 10; i++)
                {
                    try
                    {
                        if (Sign::Init())
                        {
                            if (!server->Run(ip, port))
                                printf("Server run failed\n");
                            break;
                        }
                    }
                    catch (const std::exception &e)
                    {
                        printf("Init failed: %s\n", e.what());
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            })
                .detach();
        }
        catch (const std::exception &e)
        {
            printf("Init failed: %s\n", e.what());
        }
        break;
    }
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        if (server != nullptr)
        {
            delete server;
            server = nullptr;
        }
        break;
    }
    return TRUE;
}