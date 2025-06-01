#pragma once
#include "driver/xraytubecontroller.h"
#include "service/logger.h"
#include "service/db.h"
#include "parser.h"
#include <memory>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

namespace jetfire27::Engine::Test {
    class TestServer {
    public:
        TestServer(unsigned short port, const std::string& dbPath); 
         ~TestServer();
        void Start(uint16_t port);
        void SetupHardwareInterface(boost::asio::io_context& io);
        void Run();
        void HandleSession(boost::asio::ip::tcp::socket socket); 
        void Stop();
        std::shared_ptr<jetfire27::Engine::DB::SQLiteDB> get_database();
        void update_database(std::shared_ptr<jetfire27::Engine::DB::SQLiteDB> new_db);
        
    private:
        using HttpRequest = boost::beast::http::request<boost::beast::http::string_body>;
        using HttpResponse = boost::beast::http::response<boost::beast::http::string_body>;

        void AddRoute(const std::string& path, 
            std::function<void(const HttpRequest&, HttpResponse&)> handler);
        
        void AddCommandHandlers();
        void handleCommand(const HttpRequest& req, HttpResponse& res);
        void sendSuccess(HttpResponse& res, const std::string& success_msg = "");
        void sendError(HttpResponse& res, const std::string& message);
        void start_accept();
        void handle_accept(const boost::system::error_code& ec, 
                               std::shared_ptr<boost::asio::ip::tcp::socket> socket);

        unsigned short port_;
        std::shared_ptr<DB::SQLiteDB> db_;
        boost::asio::io_context m_ioc;
        std::unique_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
        std::unordered_map<std::string, 
            std::function<void(const HttpRequest&, HttpResponse&)>> m_routes;
    };
}