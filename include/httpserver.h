// HttpServer.hpp
#pragma once
#include <boost/beast.hpp>
#include <memory>

namespace beast = boost::beast;
namespace http = beast::http;

class HttpServer : public Singleton<HttpServer> {
public:
    void start(boost::asio::io_context& io, uint16_t port) {
        m_acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(
            io, boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(), port});
        start_accept();
    }

    void add_route(const std::string& path, std::function<void(const http::request<http::string_body>&, 
                  http::response<http::string_body>&)> handler) {
        m_routes[path] = handler;
    }

private:
    void start_accept() {
        m_socket.emplace(*m_acceptor->get_io_context());
        
        m_acceptor->async_accept(*m_socket,
            [this](beast::error_code ec) {
                if (!ec) handle_request();
                start_accept();
            });
    }

    void handle_request() {
        auto req = std::make_shared<http::request<http::string_body>>();
        auto sock = std::make_shared<boost::asio::ip::tcp::socket>(std::move(*m_socket));
        
        http::async_read(*sock, m_buffer, *req,
            [this, req, sock](beast::error_code ec, size_t) {
                if (!ec) process_request(*req, *sock);
            });
    }

    void process_request(const http::request<http::string_body>& req, 
                       boost::asio::ip::tcp::socket& socket) {
        http::response<http::string_body> res{http::status::ok, req.version()};
        
        auto it = m_routes.find(req.target().to_string());
        if (it != m_routes.end()) {
            it->second(req, res);
        } else {
            res.result(http::status::not_found);
            res.body() = "Not Found";
        }
        
        http::write(socket, res);
    }

    std::unique_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
    std::optional<boost::asio::ip::tcp::socket> m_socket;
    beast::flat_buffer m_buffer;
    std::unordered_map<std::string, 
        std::function<void(const http::request<http::string_body>&, 
                         http::response<http::string_body>&)>> m_routes;
};