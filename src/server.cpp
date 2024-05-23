#include "server.h"
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/writer.h"

#include <thread>

std::string ConstructResponse(const std::string &sign, const std::string &extra, const std::string &token) {
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

Server::Server(int port) {
    printf("Start server on port: %d\n", port);
    std::thread(&Server::Init, this, port)
        .detach();
}

void Server::Init(int port) {
    try {
        svr.Post("/sign", [this](const httplib::Request &req, httplib::Response &res) {
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

        svr.Get("/sign", [this](const httplib::Request &req, httplib::Response &res) {
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

        svr.Get("/ping", [](const httplib::Request &req, httplib::Response &res) {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

            writer.StartObject();
            writer.Key("code");
            writer.Int(0);
            writer.EndObject();

            res.set_content(buffer.GetString(), "application/json");
            });

        svr.Get("/count", [this](const httplib::Request &req, httplib::Response &res) {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

            writer.StartObject();
            writer.Key("count");
            writer.String(std::to_string(counter.load()).c_str());
            writer.EndObject();

            res.set_content(buffer.GetString(), "application/json"); 
        });

        bool ret = svr.listen("127.0.0.1", port); // crach here
    }
    catch(std::exception &e) {
        printf("Server init failed: %s\n", e.what());
    }
}