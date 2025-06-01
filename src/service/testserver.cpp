#include "service/testserver.h"
#include "service/testrecord.h"

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/json.hpp>
#include <fmt/format.h>
#include <filesystem>
#include <boost/asio/ip/tcp.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
using tcp = asio::ip::tcp;

using jetfire27::Engine::Test::TestRecord;
using jetfire27::Engine::Test::TestServer;

namespace fmt
{
    template <>
    struct formatter<boost::asio::ip::tcp::endpoint>
    {
        constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const boost::asio::ip::tcp::endpoint &ep, FormatContext &ctx) const
        {
            return format_to(ctx.out(), "{}:{}", ep.address().to_string(), ep.port());
        }
    };
} // namespace fmt

namespace jetfire27::Engine::JsonParser
{
    template <>
    std::string Parser<TestRecord>::Marshall(const TestRecord &o)
    {
        json::object j;
        j["id"] = o.id;
        j["name"] = o.name;
        return json::serialize(j);
    }

    template <>
    TestRecord Parser<TestRecord>::UnMarshall(const std::string &s)
    {
        auto o = json::parse(s).as_object();
        return {
            static_cast<int>(o["id"].as_int64()),
            std::string(o["name"].as_string())};
    }
}

TestServer::TestServer(unsigned short port, const std::string &dbPath)
    : port_(port),

      m_ioc(),
      m_acceptor(std::make_unique<tcp::acceptor>(m_ioc, tcp::endpoint(tcp::v4(), port)))
{
    jetfire27::Engine::Logging::Logger::GetInstance().Info("Creating server on port {}", port);
    try
    {
        db_ = std::make_shared<DB::SQLiteDB>(dbPath);
        db_->Execute(
            "CREATE TABLE IF NOT EXISTS xray_settings ("
            "key TEXT PRIMARY KEY,"
            "value TEXT);"
            "CREATE TABLE IF NOT EXISTS exposure_log ("
            "id INTEGER PRIMARY KEY,"
            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "voltage INTEGER,"
            "current REAL,"
            "duration INTEGER,"
            "mode TEXT,"
            "params TEXT);" // JSON с дополнительными параметрами

            "CREATE TABLE IF NOT EXISTS system_events ("
            "id INTEGER PRIMARY KEY,"
            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "event_type TEXT,"
            "details TEXT);");
#ifdef _WIN32
        const char *default_com = "COM1";
#else
        const char *default_com = "/dev/ttyUSB0";
#endif

        db_->Execute(
            "INSERT OR IGNORE INTO xray_settings (key, value) "
            "VALUES ('com_port', '" +
            std::string(default_com) + "')");
        AddCommandHandlers();
        jetfire27::Engine::Logging::Logger::GetInstance().Info("Initialized TestServer on port {}", port);
    }
    catch (const std::exception &e)
    {
        jetfire27::Engine::Logging::Logger::GetInstance().Error("DB error: {}", e.what());
        throw;
    }
    jetfire27::Engine::Logging::Logger::GetInstance().Info("server on port {} created success", port);
}

void TestServer::SetupHardwareInterface(boost::asio::io_context &io)
{
    try
    {
        XRayTubeController::instance().init(io, get_database());
    }
    catch (const std::exception &e)
    {
        Logging::Logger::GetInstance().Critical("Hardware init failed: {}", e.what());
        throw;
    }
}

void TestServer::AddRoute(
    const std::string &path,
    std::function<void(const HttpRequest &, HttpResponse &)> handler)
{
    m_routes.emplace(path, handler);
}

void TestServer::start_accept()
{
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_ioc);
    m_acceptor->async_accept(*socket,
                             [this, socket](const boost::system::error_code &ec)
                             {
                                 handle_accept(ec, socket);
                             });
}

void TestServer::handle_accept(const boost::system::error_code &ec,
                               std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    if (!ec)
    {
        std::thread(&TestServer::HandleSession, this, std::move(*socket)).detach();
    }
    start_accept();
}

void TestServer::Run()
{
    try
    {
        for (;;)
        {
            auto sock = boost::asio::ip::tcp::socket(m_ioc);
            m_acceptor->accept(sock);
            HandleSession(std::move(sock));
        }
    }
    catch (const std::exception &e)
    {
        Logging::Logger::GetInstance().Critical("Server stopped critical: {}", e.what());
    }
}

void TestServer::Stop()
{
    boost::system::error_code ec;
    if (m_acceptor)
    {
        m_acceptor->close(ec);
    }

    m_ioc.stop();
}

std::shared_ptr<jetfire27::Engine::DB::SQLiteDB> TestServer::get_database()
{
    return std::atomic_load(&db_);
}

void TestServer::update_database(std::shared_ptr<DB::SQLiteDB> new_db)
{
    std::atomic_store(&db_, new_db);
}

TestServer::~TestServer() { Stop(); }

void TestServer::Start(uint16_t port)
{
    try
    {
        tcp::endpoint endpoint(tcp::v4(), port);
        m_acceptor->open(endpoint.protocol());
        m_acceptor->set_option(tcp::acceptor::reuse_address(true));
        m_acceptor->bind(endpoint);
        m_acceptor->listen();
        jetfire27::Engine::Logging::Logger::GetInstance().Info("Server listening on port {}", port);
    }
    catch (const boost::system::system_error &e)
    {
        jetfire27::Engine::Logging::Logger::GetInstance().Critical("Port {} unavailable: {}", port, e.what());
        throw;
    }
    Run();
}

void TestServer::sendSuccess(HttpResponse &res)
{
    res.result(http::status::ok);
    res.body() = R"({"status":"success"})";
    res.prepare_payload();
}

void TestServer::sendError(HttpResponse &res, const std::string &message)
{
    res.result(http::status::bad_request);
    json::object error;
    error["error"] = message;
    res.body() = json::serialize(error);
    res.prepare_payload();
}

void TestServer::AddCommandHandlers()
{
    // Эндпоинт /api/stats
    AddRoute("/api/stats", [this](const HttpRequest &req, HttpResponse &res)
             {
        json::object stats;
        db_->Execute(
            "SELECT COUNT(*) FROM exposure_log",
            [](void* data, int, char** vals, char**) {
                (*static_cast<json::object*>(data))["total_exposures"] = std::stoi(vals[0]);
                return 0;
            }, &stats);
        
        db_->Execute(
            "SELECT MAX(timestamp) FROM system_events WHERE event_type='ERROR'",
            [](void* data, int, char** vals, char**) {
                if(vals[0]) 
                    (*static_cast<json::object*>(data))["last_error"] = vals[0];
                return 0;
            }, &stats);
        
        res.body() = json::serialize(stats); });

    // Конфигурация
    AddRoute("/api/config", [this](const HttpRequest &req, HttpResponse &res)
             {
        try {
            if(req.method() == http::verb::get) {
                json::object config;
                db_->Execute("SELECT key, value FROM xray_settings",
                    [](void* data, int argc, char** argv, char** cols) {
                        auto cfg = static_cast<json::object*>(data);
                        (*cfg)[argv[0]] = argv[1];
                        return 0;
                    }, &config);
                res.body() = json::serialize(config);
            }
            else if(req.method() == http::verb::post) {
                auto j = json::parse(req.body());
                for(const auto& item : j.as_object()) {
                    if (item.key() == "com_port") {
                std::string port = item.value().as_string().c_str();
#ifdef _WIN32
                if (port.find("COM") != 0 || port.size() < 4) {
                    throw std::runtime_error("Invalid COM port format. Use COMx");
                }
#endif
            }
                    db_->Execute(fmt::format(
                        "INSERT OR REPLACE INTO xray_settings VALUES('{}','{}')",
                        item.key_c_str(),
                        item.value().as_string().c_str()));
                }
                sendSuccess(res);
            }
        } catch(const std::exception& e) {
            sendError(res, e.what());
        } });

    // Экспозиция
    AddRoute("/api/exposure/now", [this](const HttpRequest &req, HttpResponse &res)
             {
    try {
        auto params = json::parse(req.body());
        const uint32_t duration = params.at("duration").as_int64();
        const std::string mode = params.at("mode").as_string().c_str();
        
        // Проверка режима
        if (mode != "test" && mode != "standard" && mode != "pulsed")
            throw std::runtime_error("Invalid exposure mode");
        
        // Запуск экспозиции
        XRayTubeController::instance().start_exposure(duration);
        
        jetfire27::Engine::Logging::Logger::GetInstance().Info(
            "Exposure started: {}ms, mode: {}", duration, mode);
        
        sendSuccess(res);
    } catch(const std::exception& e) {
        sendError(res, e.what());
    } });

    AddRoute("/api/voltage", [this](const HttpRequest &req, HttpResponse &res)
             {
    try {
        auto params = json::parse(req.body());
        const uint16_t voltage = params.at("voltage").as_int64();
        
        if (voltage < 10 || voltage > 150)
            throw std::runtime_error("Invalid voltage (10-150 kV)");
            
        XRayTubeController::instance().set_voltage(voltage);
        sendSuccess(res);
    } catch(const std::exception& e) {
        sendError(res, e.what());
    } });

    AddRoute("/api/current", [this](const HttpRequest &req, HttpResponse &res)
             {
    try {
        auto params = json::parse(req.body());
        const float current = params.at("current").as_double();
        
        if (current < 0.01f || current > 0.4f)
            throw std::runtime_error("Invalid current (0.01-0.4 mA)");
            
        XRayTubeController::instance().set_current(current);
        sendSuccess(res);
    } catch(const std::exception& e) {
        sendError(res, e.what());
    } });

    // Аварийная остановка
    AddRoute("/api/emergency_stop", [this](const HttpRequest &req, HttpResponse &res)
             {
        XRayTubeController::instance().emergency_stop();
        sendSuccess(res); });

    // Статус системы
    AddRoute("/api/status", [this](const HttpRequest &req, HttpResponse &res)
             {
    try {
        auto status = XRayTubeController::instance().get_status();
        json::object jstatus;
        jstatus["voltage_kv"] = status.voltage_kv;
        jstatus["current_ma"] = status.current_ma;
        jstatus["exposure_active"] = status.exposure_active;
        jstatus["filament_on"] = status.filament_on;
        jstatus["error_state"] = status.error_state;
        jstatus["last_error"] = XRayTubeController::instance().last_error();
        res.body() = json::serialize(jstatus);
        res.result(http::status::ok);
    } catch(const std::exception& e) {
        sendError(res, "Failed to get status: " + std::string(e.what()));
    } });

    // Логи экспозиции
    AddRoute("/api/logs/exposure", [this](const HttpRequest &req, HttpResponse &res)
             {
        try {
            int limit = 100;
            int offset = 0;
            
            // Парсинг параметров запроса
            if(auto param = req.find("limit"); param != req.end())
                limit = std::stoi(param->value());
            
            if(auto param = req.find("offset"); param != req.end())
                offset = std::stoi(param->value());

            json::array logs;
            db_->Execute(fmt::format(
                "SELECT timestamp, voltage, current, duration, mode "
                "FROM exposure_log ORDER BY id DESC LIMIT {} OFFSET {}", 
                limit, offset),
                [](void* data, int argc, char** argv, char** cols) {
                    auto arr = static_cast<json::array*>(data);
                    json::object log;
                    log["timestamp"] = argv[0];
                    log["voltage"] = std::stoi(argv[1]);
                    log["current"] = std::stof(argv[2]);
                    log["duration"] = std::stoi(argv[3]);
                    log["mode"] = argv[4];
                    arr->push_back(log);
                    return 0;
                }, &logs);

            res.body() = json::serialize(logs);
        } catch(const std::exception& e) {
            sendError(res, e.what());
        } });

    // Системные логи
    AddRoute("/api/logs/system", [this](const HttpRequest &req, HttpResponse &res)
             {
    try {
        // Получаем директорию логов из Logger
        std::string logDir = jetfire27::Engine::Logging::Logger::GetInstance().GetLogDirectory();

        // Получаем текущую дату
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);

        // Формируем имя лог-файла с учетом текущей даты
        std::ostringstream filenameStream;
        filenameStream << "jet_service_" << std::put_time(&tm, "%Y-%m-%d") << ".log";
        std::filesystem::path logFile = std::filesystem::path(logDir) / filenameStream.str();

        if (!std::filesystem::exists(logFile)) {
            // Если файл не найден — отвечаем 404
            res.result(boost::beast::http::status::not_found);
            res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
            res.body() = R"({"error":"Log file not found"})";
            res.prepare_payload();
            return;
        }

        // Открываем файл и читаем всё в строку
        std::ifstream ifs(logFile, std::ios::in);
        if (!ifs.is_open()) {
            throw std::runtime_error("Unable to open log file");
        }
        std::ostringstream ss;
        ss << ifs.rdbuf();
        std::string contents = ss.str();

        // Отправляем ответ как plain-text
        res.set(boost::beast::http::field::content_type, "text/plain; charset=utf-8");
        res.result(boost::beast::http::status::ok);
        res.body() = contents;
        res.prepare_payload();
    }
    catch(const std::exception& e) {
        sendError(res, e.what());
    } });

    // Перезапуск драйвера
    AddRoute("/api/driver/restart", [this](const HttpRequest &req, HttpResponse &res)
             {
        try {
            if(XRayTubeController::instance().is_exposure_active())
                throw std::runtime_error("Cannot restart during exposure");

            XRayTubeController::instance().restart_driver();
            
            // Логируем событие
            db_->Execute(fmt::format(
                "INSERT INTO system_events (event_type, details) VALUES "
                "('DRIVER_RESTART', 'Driver restarted at {}')",
                std::time(nullptr)));
            
            sendSuccess(res);
        } catch(const std::exception& e) {
            sendError(res, e.what());
        } });

    // Статус драйвера
    AddRoute("/api/driver/status", [this](const HttpRequest &req, HttpResponse &res)
             {
        try {
        json::object status;
        status["connected"] = XRayTubeController::instance().is_connected();
        status["exposure_active"] = XRayTubeController::instance().is_exposure_active();
        status["last_error"] = XRayTubeController::instance().last_error();
        res.body() = json::serialize(status);
    } catch(const std::exception& e) {
            sendError(res, e.what());
        }});

    // Проверка соединения
    AddRoute("/api/connection/test", [this](const HttpRequest &req, HttpResponse &res)
             {
        try {
            bool success = XRayTubeController::instance().test_connection();
            json::object response;
            response["status"] = success ? "OK" : "ERROR";
            response["response_time"] = XRayTubeController::instance().last_ping_time().count();
            res.body() = json::serialize(response);
        } catch(const std::exception& e) {
            sendError(res, e.what());
        } });
}

void TestServer::HandleSession(boost::asio::ip::tcp::socket socket)
{
    try
    {
        auto &log = jetfire27::Engine::Logging::Logger::GetInstance();
        const auto &ep = socket.remote_endpoint();
        log.Info("New connection from {}:{}", ep.address().to_string(), ep.port());
        beast::tcp_stream stream(std::move(socket));
        beast::flat_buffer buf;
        http::request<http::string_body> req;
        http::read(stream, buf, req);

        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());

        std::string target_path = std::string(req.target());
        log.Info("Incoming response: {}", target_path);
        size_t pos = target_path.find('?');
        if (pos != std::string::npos)
        {
            target_path = target_path.substr(0, pos);
        }

        // Поиск зарегистрированного обработчика
        auto it = m_routes.find(target_path);
        if (it != m_routes.end())
        {
            it->second(req, res); // Вызов обработчика
        }
        else
        {
            res.result(http::status::not_found);
            res.body() = R"({"error":"endpoint not found"})";
        }

        // Отправка ответа
        res.prepare_payload();
        http::write(stream, res);

        stream.socket().shutdown(tcp::socket::shutdown_send);
    }
    catch (const std::exception &e)
    {
        jetfire27::Engine::Logging::Logger::GetInstance().Error("Session error: {}", e.what());
    }
}
