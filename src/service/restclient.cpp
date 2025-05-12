#include "restclient.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/version.hpp>
#include <stdexcept>
#include <string>

using namespace boost;
using tcp  = asio::ip::tcp;
namespace http = beast::http;
namespace json = boost::json;

namespace {

std::string do_request_no_body(
    http::verb method,
    const std::string& host,
    int port,
    const std::string& target,
    const std::string& auth_token
) {
    asio::io_context ioc;
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    auto const results = resolver.resolve(host, std::to_string(port));
    stream.connect(results);

    http::request<http::empty_body> req{method, target, 11};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    if (!auth_token.empty()) {
        req.set(http::field::authorization, "Bearer " + auth_token);
    }
    req.prepare_payload();

    http::write(stream, req);
    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(stream, buffer, res);
    stream.socket().shutdown(tcp::socket::shutdown_both);

    if (res.result() != http::status::ok) {
        throw std::runtime_error(
            "HTTP error " + std::to_string(res.result_int())
            + ": " + std::string(res.reason())
        );
    }
    return res.body();
}

std::string do_request_with_body(
    http::verb method,
    const std::string& host,
    int port,
    const std::string& target,
    const std::string& auth_token,
    const std::string& body_json
) {
    asio::io_context ioc;
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    auto const results = resolver.resolve(host, std::to_string(port));
    stream.connect(results);

    http::request<http::string_body> req{method, target, 11};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::content_type, "application/json");
    if (!auth_token.empty()) {
        req.set(http::field::authorization, "Bearer " + auth_token);
    }
    req.body() = body_json;
    req.prepare_payload();

    http::write(stream, req);
    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(stream, buffer, res);
    stream.socket().shutdown(tcp::socket::shutdown_both);

    if (res.result() != http::status::ok) {
        throw std::runtime_error(
            "HTTP error " + std::to_string(res.result_int())
            + ": " + std::string(res.reason())
        );
    }
    return res.body();
}

} // namespace

jetfire27::Engine::REST::RestClient::RestClient(
    const std::string& base_url,
    int port
) : base_url_(base_url), port_(port) {}

std::string jetfire27::Engine::REST::RestClient::Get(const std::string& target) {
    return do_request_no_body(
        http::verb::get,
        base_url_,
        port_,
        target,
        auth_token_
    );
}

std::string jetfire27::Engine::REST::RestClient::Delete(const std::string& target) {
    return do_request_no_body(
        http::verb::delete_,
        base_url_,
        port_,
        target,
        auth_token_
    );
}

std::string jetfire27::Engine::REST::RestClient::Post(
    const std::string& target,
    const json::value& body
) {
    return do_request_with_body(
        http::verb::post,
        base_url_,
        port_,
        target,
        auth_token_,
        json::serialize(body)
    );
}

std::string jetfire27::Engine::REST::RestClient::Put(
    const std::string& target,
    const json::value& body
) {
    return do_request_with_body(
        http::verb::put,
        base_url_,
        port_,
        target,
        auth_token_,
        json::serialize(body)
    );
}
