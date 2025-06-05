#include "service/testserver.h"
#include "service/daemonizer.h"
#include "service/logger.h"
#include "service/filesystem_utils.h"
#include <exception>
#include <memory>
#include <boost/asio.hpp>
#include <csignal>
#include <atomic>

#include "driver/recognition_provider.h"
#include <iostream>
#include <fstream>

// Вспомогательная функция для записи PNG-байтов в файл:
static void SavePNG(const std::vector<unsigned char>& png_bytes, const std::string& filename) {
    std::ofstream ofs(filename, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(png_bytes.data()), png_bytes.size());
    ofs.close();
    if (!ofs) {
        std::cerr << "Ошибка при записи файла: " << filename << "\n";
    }
}

int RecTest() {
    try {
        RecognitionProvider prov;
        std::cout << "RecognitionProvider создан.\n";

        // 1) Тест estimate_body_build()
        std::cout << "Запуск estimate_body_build()...\n";
        BodyBuild build = prov.estimate_body_build();
        std::cout << "Результат: ";
        if (build == BodyBuild::SMALL)  std::cout << "SMALL\n";
        if (build == BodyBuild::MEDIUM) std::cout << "MEDIUM\n";
        if (build == BodyBuild::LARGE)  std::cout << "LARGE\n";

        // Сохраним 4 изображения этапов в файлы
        auto imgs1 = prov.GetImages();
        if (imgs1.size() == 4) {
            for (size_t i = 0; i < imgs1.size(); ++i) {
                std::string fname = "build_stage_" + std::to_string(i) + ".png";
                SavePNG(imgs1[i], fname);
                std::cout << "  → " << fname << "\n";
            }
        }

        // 2) Тест estimate_body_thickness()
        std::cout << "Запуск estimate_body_thickness()...\n";
        float thickness = prov.estimate_body_thickness();
        std::cout << "Примерная толщина (ширина пикселей): " << thickness << "\n";

        // Сохраним следующие 4 изображения
        auto imgs2 = prov.GetImages();
        if (imgs2.size() == 4) {
            for (size_t i = 0; i < imgs2.size(); ++i) {
                std::string fname = "thickness_stage_" + std::to_string(i) + ".png";
                SavePNG(imgs2[i], fname);
                std::cout << "  → " << fname << "\n";
            }
        }

        // 3) Запуск GetVideo (Esc для выхода)
        std::cout << "Запуск видеопотока. Нажмите Esc, чтобы выйти.\n";
        prov.GetVideo();
        std::cout << "GetVideo завершён.\n";
    }
    catch (const std::exception& ex) {
        std::cerr << "Ошибка: " << ex.what() << "\n";
        return -1;
    }

    return 0;
}


int main_function();

#ifdef _WIN32
#include <windows.h>

#ifdef IS_GUI_APP
  int WINAPI WinMain(
      HINSTANCE hInstance,
      HINSTANCE hPrevInstance,
      LPSTR lpCmdLine,
      int nCmdShow)
  {
      return main_function();
  }
  #else
  int main() {
      return main_function();
  }
  #endif

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND CreateHiddenWindow()
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "XRayDriverWindowClass";
    RegisterClass(&wc);

    return CreateWindowEx(
        0,
        "XRayDriverWindowClass",
        "XRayDriver",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr);
}
#endif

#define _SILENCE_CXX20_OLD_SHARED_PTR_ATOMIC_SUPPORT_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX20_DEPRECATION_WARNINGS

std::function<void()> shutdown_handler;
void signal_handler(int) { shutdown_handler(); }
std::atomic<bool> running_{true};

void out_handler(int signal)
{
    if (signal == SIGINT)
    {
        jetfire27::Engine::Logging::Logger::GetInstance().Info("Pushed Ctrl+C, exiting from program...");
        running_ = false;
    }
}

#ifndef _WIN32
int main() {
    return main_function();
}
#endif

int main_function()
{
    using namespace jetfire27::Engine;

    std::set_terminate([]() {
        Logging::Logger::GetInstance().Critical("Unhandled exception detected!");
        std::exit(EXIT_FAILURE);
    });

#ifdef _WIN32
    HWND hWnd = nullptr;
    #ifdef IS_GUI_APP
        hWnd = CreateHiddenWindow();
        ShowWindow(hWnd, SW_HIDE);
    #endif
#endif
    auto& log = Logging::Logger::GetInstance();
    log.Initialize(jetfire27::Engine::FilesystemUtils::GetLogDirectory());
    log.Info("============Starting new session of XRAY Dynamic Driver============");
    std::signal(SIGINT, out_handler);
    
    //!!!!!
    //RecTest();
    try
    {
        if (Daemonizer::IsSingleInstance())
        {
            Daemonizer::Setup(jetfire27::Engine::FilesystemUtils::getExecutablePath(), Mode::Service);
        }

        boost::asio::io_context io;
        boost::asio::signal_set signals(io, SIGINT, SIGTERM);
        std::shared_ptr<Test::TestServer> server = std::make_shared<Test::TestServer>(8080, jetfire27::Engine::FilesystemUtils::GetDBPath());
        shutdown_handler = [&]
        {
            io.stop();
            server->Stop();
        };

        signals.async_wait([&](auto, auto)
                           { shutdown_handler(); });
        std::thread server_thread([&]
                                  { 
        server->SetupHardwareInterface(io);
        server->Run(); });
        io.run();
        server_thread.join();

#ifdef _WIN32
    #ifdef IS_GUI_APP
            MSG msg;
            while (GetMessage(&msg, nullptr, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
    #else
            while (running_)
            {
            }; // Консольная версия
    #endif
#else
        while (running_)
        {
        }; // Linux/macOS версия
#endif
    }
    catch (const std::exception &e)
    {
        Logging::Logger::GetInstance().Critical("XRAY Driver initialization failed: {}", e.what());
        return 1;
    }

    return 0;
}