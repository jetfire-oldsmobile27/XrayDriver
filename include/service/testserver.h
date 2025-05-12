#pragma once
#include "driver/xraytubecontroller.h"
#include "service/logger.h"
#include "service/db.h"
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

namespace jetfire27::Engine::Test {
    class TestServer {
    public:
        TestServer(unsigned short port, const std::string& dbPath); // Исправлен конструктор
        void SetupHardwareInterface(boost::asio::io_context& io);
        void Run();
        void HandleSession(boost::asio::ip::tcp::socket socket); // Пространство имён + ;
        void Stop();
        
    private:
        using HttpRequest = boost::beast::http::request<boost::beast::http::string_body>;
        using HttpResponse = boost::beast::http::response<boost::beast::http::string_body>;

        void AddRoute(const std::string& path, 
            std::function<void(const HttpRequest&, HttpResponse&)> handler);
        
        void AddCommandHandlers();
        void handleCommand(const HttpRequest& req, HttpResponse& res);
        void sendSuccess(HttpResponse& res);
        void sendError(HttpResponse& res, const std::string& message);

        unsigned short port_;
        DB::SQLiteDB db_;
        boost::asio::io_context m_ioc;
        std::unique_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
        std::unordered_map<std::string, 
            std::function<void(const HttpRequest&, HttpResponse&)>> m_routes;
    };
}