#include "testserver.h"
#include "testrecord.h"

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/json.hpp>
#include <fmt/format.h>
#include <boost/asio/ip/tcp.hpp>

namespace asio  = boost::asio;
namespace beast = boost::beast;
namespace http  = beast::http;
namespace json  = boost::json;
using   tcp    = asio::ip::tcp;

using jetfire27::Engine::Test::TestServer;
using jetfire27::Engine::Test::TestRecord;

namespace fmt {
template <>
struct formatter<boost::asio::ip::tcp::endpoint> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    
    template <typename FormatContext>
    auto format(const boost::asio::ip::tcp::endpoint& ep, FormatContext& ctx) const {
        return format_to(ctx.out(), "{}:{}", ep.address().to_string(), ep.port());
    }
};
} // namespace fmt

namespace jetfire27::Engine::JsonParser {
    template<>
    inline std::string Parser<TestRecord>::Marshall(const TestRecord& o) {
        json::object j;
        j["id"]   = o.id;
        j["name"] = o.name;
        return json::serialize(j);
    }
    template<>
    inline TestRecord Parser<TestRecord>::UnMarshall(const std::string& s) {
        auto o = json::parse(s).as_object();
        return TestRecord{
            int(o["id"].as_int64()),
            std::string(o["name"].as_string())
        };
    }
}

TestServer::TestServer(unsigned short port, const std::string& dbPath)
    : port_(port), db_(dbPath), ioc_(), acceptor_{ioc_, {tcp::v4(), port_}}
{
    

    try {
        db_.Execute(
            "CREATE TABLE IF NOT EXISTS test ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "name TEXT NOT NULL);"
        );
        jetfire27::Engine::Logging::Logger::GetInstance().Info("Initialized TestServer on port {}", port);
    } catch (const std::exception& e) {
        jetfire27::Engine::Logging::Logger::GetInstance().Error("DB error: {}", e.what());
        throw;
    }
}

TestServer::~TestServer() { Stop(); }

void TestServer::Run() {
    try {
        for (;;) {
            tcp::socket sock{ioc_};
            acceptor_.accept(sock);           
            HandleSession(std::move(sock));
        }
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Server stopped critical: %s\n", e.what());
    }
}

void TestServer::Stop() {
    boost::system::error_code ec;
    acceptor_.close(ec);
    ioc_.stop();
}

void TestServer::HandleSession(tcp::socket socket) {
    const auto& ep = socket.remote_endpoint();
    jetfire27::Engine::Logging::Logger::GetInstance().Info("New connection from {}:{}", ep.address().to_string(), ep.port());
    beast::tcp_stream stream(std::move(socket));
    beast::flat_buffer buf;
    http::request<http::string_body> req;
    http::read(stream, buf, req);

    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());

    if (req.method() == http::verb::get && req.target() == "/items") {
        // C-style callback for sqlite3_exec
        struct CB { static int f(void* d,int c,char**v,char**){
            auto vec = static_cast<std::vector<TestRecord>*>(d);
            vec->push_back({ std::stoi(v[0]), v[1] });
            return 0;
        }};
        std::vector<TestRecord> vec;
        db_.Execute("SELECT id,name FROM test;", CB::f, &vec);

        // avoid match with:contentReference[oaicite:11]{index=11}
        jetfire27::Engine::JsonParser::Parser<TestRecord> parser;
        json::array arr;
        for (auto& r : vec) {
            arr.push_back(json::parse(parser.Marshall(r)));
        }
        res.body() = json::serialize(arr);

    } else if (req.method() == http::verb::post && req.target() == "/items") {
        jetfire27::Engine::JsonParser::Parser<TestRecord> parser;
        auto rec = parser.UnMarshall(req.body());
        db_.Execute("INSERT INTO test(name) VALUES('" + rec.name + "');");
        res.body() = R"({"status":"ok"})";

    } else {
        res.result(http::status::bad_request);
        res.body() = R"({"error":"bad request"})";
    }

    res.prepare_payload();
    http::write(stream, res);
    stream.socket().shutdown(tcp::socket::shutdown_send);
}
