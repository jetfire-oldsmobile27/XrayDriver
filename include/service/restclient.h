#pragma once
#include "logger.h"
#include <string>
#include <boost/json.hpp>

namespace jetfire27::Engine::REST {

class RestClient {
public:
    RestClient(const std::string& base_url, int port = 80);
    
    void SetAuthToken(const std::string& token) { auth_token_ = token; }
    void ClearAuthToken() { auth_token_.clear(); }

    std::string Get(const std::string& target);
    std::string Post(const std::string& target, const boost::json::value& body);
    std::string Put(const std::string& target, const boost::json::value& body);
    std::string Delete(const std::string& target);

private:
    std::string build_full_url(const std::string& target);

    std::string base_url_;
    int port_;
    std::string auth_token_;
};

} // namespace jetfire27::Engine::REST