#include "sign.h"
#include "server.h"
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/writer.h"

#include <thread>

Sign sign;

Server::Server(int port)
{
    svr.Post("/sign", [](const httplib::Request &req, httplib::Response &res)
             { 
        try
        {
            rapidjson::Document doc;
            doc.Parse(req.body.c_str(), req.body.size());

            std::string_view cmd = doc["cmd"].GetString();
            std::string_view src = doc["src"].GetString();
            int seq = doc["seq"].GetInt64();

            auto [signDataHex, extraDataHex, tokenDataHex] = sign.Call(cmd, src, seq);

            // Construct response
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            writer.StartObject();
            writer.Key("sign");
            writer.String(signDataHex.c_str());
            writer.Key("extra");
            writer.String(extraDataHex.c_str());
            writer.Key("token");
            writer.String(tokenDataHex.c_str());
            writer.EndObject();

            res.set_content(buffer.GetString(), "application/json");
        }
        catch (...)
        {
            res.set_content("Bad Request", "text/plain");
            res.status = httplib::StatusCode::BadRequest_400;
        } });

    std::thread([this, port]
                { svr.listen("0.0.0.0", port); }).detach();
}