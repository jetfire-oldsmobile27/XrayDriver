// Daemonizer.hpp
#pragma once
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <system_error>

class Daemonizer {
public:
    static void daemonize() {
        pid_t pid = fork();
        
        if (pid < 0)
            throw std::system_error(errno, std::system_category(), "Fork failed");
        
        if (pid > 0) // Родительский процесс
            exit(EXIT_SUCCESS);
        
        if (setsid() < 0)
            throw std::system_error(errno, std::system_category(), "setsid failed");
        
        umask(0);
        
        if (chdir("/") < 0)
            throw std::system_error(errno, std::system_category(), "chdir failed");
        
        redirect_standard_fds();
    }

private:
    static void redirect_standard_fds() {
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        
        if (open("/dev/null", O_RDONLY) < 0)
            throw std::system_error(errno, std::system_category(), "stdin redirect failed");
        
        if (open("/dev/null", O_WRONLY) < 0)
            throw std::system_error(errno, std::system_category(), "stdout redirect failed");
        
        if (open("/dev/null", O_RDWR) < 0)
            throw std::system_error(errno, std::system_category(), "stderr redirect failed");
    }
};