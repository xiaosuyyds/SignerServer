#include "sign.h"
#include "server.h"
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/writer.h"

#include <thread>

Sign sign;

// HTTP server event handler function
void ev_handler(struct mg_connection *c, int ev, void *ev_data)
{
    if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = reinterpret_cast<mg_http_message *>(ev_data);
        if (mg_match(hm->uri, mg_str("/sign"), NULL))
        {
            try
            {
                rapidjson::Document doc;
                doc.Parse(hm->body.buf, hm->body.len);

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

                mg_http_reply(c, 200, NULL, buffer.GetString());
            }
            catch (...)
            {
                mg_http_reply(c, 400, NULL, "400 Bad Request");
                return;
            }
        }
        else
        {
            mg_http_reply(c, 404, NULL, "404 Not Found");
        }

        // Log request
        // MG_INFO(("%.*s %.*s %lu -> %.*s %lu", hm->method.len, hm->method.buf,
        //          hm->uri.len, hm->uri.buf, hm->body.len, 3, c->send.buf + 9,
        //          c->send.len));
    }
}

Server::Server(int port)
{
    char url[32];
    snprintf(url, sizeof(url), "http://0.0.0.0:%d", port);

    mg_mgr_init(&mgr);                           // Initialise event manager
    mg_http_listen(&mgr, url, ev_handler, NULL); // Setup listener

    // new thread to loop
    std::thread t(&Server::Loop, this);
    t.detach();
}

void Server::Loop()
{
    for (;;)
    { // Run an infinite event loop
        mg_mgr_poll(&mgr, 1000);
    }
}
