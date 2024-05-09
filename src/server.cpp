#include "server.h"
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/writer.h"

#include <thread>


Server::Server(int port)
{
    std::atomic<uint64_t> counter(0);

    svr.Post("/sign", [this, &counter](const httplib::Request &req, httplib::Response &res)
    {
        try {
            rapidjson::Document doc;
            doc.Parse(req.body.c_str(), req.body.size());

            std::string_view cmd = doc["cmd"].GetString();
            std::string_view src = doc["src"].GetString();
            int seq = doc["seq"].GetInt();

            auto [signDataHex, extraDataHex, tokenDataHex] = sign.Call(cmd, src, seq);
            std::string buffer = ConstructResponse(signDataHex, extraDataHex, tokenDataHex);
            counter++;

            res.set_content(buffer, "application/json");
        }
        catch (...) {
            res.set_content("Bad Request", "text/plain");
            res.status = httplib::StatusCode::BadRequest_400;
        }
    });

    svr.Get("/sign", [this, &counter](const httplib::Request &req, httplib::Response &res)
    {
        try {
            std::string cmd = req.get_param_value("cmd");
            std::string src = req.get_param_value("src");
            int seq = std::stoi(req.get_param_value("seq"));

            auto [signDataHex, extraDataHex, tokenDataHex] = sign.Call(cmd, src, seq);
            std::string buffer = ConstructResponse(signDataHex, extraDataHex, tokenDataHex);
            counter++;

            res.set_content(buffer, "application/json");
        }
        catch (...) {
            res.set_content("Bad Request", "text/plain");
            res.status = httplib::StatusCode::BadRequest_400;
        }
    });

    svr.Get("/ping", [](const httplib::Request &req, httplib::Response &res)
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        writer.StartObject();
        writer.Key("code");
        writer.Int(0);
        writer.EndObject();

        res.set_content(buffer.GetString(), "application/json");
    });

    svr.Get("/count", [&counter](const httplib::Request &req, httplib::Response &res)
    {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        writer.StartObject();
        writer.Key("count");
        writer.String(std::to_string(counter.load()).c_str());
        writer.EndObject();

        res.set_content(buffer.GetString(), "application/json");
    });

    std::thread([this, port]{ svr.listen("0.0.0.0", port); }).detach();
}

std::string Server::ConstructResponse(const std::string &sign, const std::string &extra, const std::string &token) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    writer.StartObject();
    writer.Key("value");
    writer.StartObject();
    writer.Key("sign");
    writer.String(sign.c_str());
    writer.Key("extra");
    writer.String(extra.c_str());
    writer.Key("token");
    writer.String(token.c_str());
    writer.EndObject();
    writer.EndObject();

    return buffer.GetString();
}