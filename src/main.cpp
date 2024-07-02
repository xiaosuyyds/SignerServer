#include "server.h"

#include "../include/rapidjson/document.h"

#include <fstream>
#include <iostream>

Server *server = nullptr;

void init()
{
    try
    {
#if defined(_WIN_PLATFORM_)
        std::string version = "9.9.12-25300";
        try
        {
            std::ifstream versionConfig("resources\\app\\versions\\config.json");
            if (versionConfig.is_open())
            {
                std::string versionConfigStr;
                versionConfig >> versionConfigStr;
                versionConfig.close();
                rapidjson::Document doc;
                doc.Parse(versionConfigStr.c_str(), versionConfigStr.size());
                if (doc.HasMember("curVersion") && doc["curVersion"].IsString())
                    version = doc["curVersion"].GetString();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
#elif defined(_MAC_PLATFORM_)
        std::string version = "6.9.19-16183";
#elif defined(_LINUX_PLATFORM_)
        std::string version = "3.2.9-24815";
        try
        {
            std::ifstream versionConfig("/opt/QQ/resources/app/package.json");
            if (versionConfig.is_open())
            {
                std::string versionConfigStr;
                versionConfig >> versionConfigStr;
                versionConfig.close();
                rapidjson::Document doc;
                doc.Parse(versionConfigStr.c_str(), versionConfigStr.size());
                if (doc.HasMember("version") && doc["version"].IsString())
                    version = doc["version"].GetString();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
#endif
        std::string ip = "0.0.0.0";
        int port = 8080;

        std::string default_config = R"({"ip":"0.0.0.0","port":8080})";

        rapidjson::Document doc;

        std::ifstream configFile("sign.json");
        if (!configFile.is_open())
        {
            printf("sign.json not found, use default\n");
            std::ofstream("sign.json") << default_config;
            doc.Parse(default_config.c_str(), default_config.size());
        }
        else
        {
            std::string config;
            configFile >> config;
            configFile.close();
            try
            {
                doc.Parse(config.c_str(), config.size());
            }
            catch (const std::exception &e)
            {
                printf("Parse config failed, use default: %s\n", e.what());
                doc.Parse(default_config.c_str(), default_config.size());
            }
        }

        if (doc.HasMember("ip") && doc["ip"].IsString())
            ip = doc["ip"].GetString();
        if (doc.HasMember("port") && doc["port"].IsInt())
            port = doc["port"].GetInt();
        if (doc.HasMember("version") && doc["version"].IsString())
            version = doc["version"].GetString();

        printf("Start Init server\n");
        server = new Server();
        server->Init();

        printf("Start Init sign\n");
        std::thread([=] { // Cannot use '&' capture!!!!! will cause crash
            for (int i = 0; i < 10; i++)
            {
                try
                {
                    if (Sign::Init(version))
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
}

void uninit()
{
    if (server != nullptr)
    {
        delete server;
        server = nullptr;
    }
}

#if defined(_WIN_PLATFORM_)
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        init();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        uninit();
        break;
    }
    return TRUE;
}
#elif defined(_LINUX_PLATFORM_) || defined(_MAC_PLATFORM_)
void __attribute__((constructor)) my_init(void)
{
    init();
}

void __attribute__ ((destructor)) my_fini(void)
{
    uninit();
}
#endif