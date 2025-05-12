#pragma once
#include "logger.h"
#include <string>
#include <sqlite3.h>

namespace jetfire27::Engine::DB {
    class SQLiteDB {
    public:
        SQLiteDB(const std::string& dbPath);
        ~SQLiteDB();
        void Execute(const std::string& sql);
        // для выборок:
        void Execute(const std::string& sql,
                     int (*callback)(void*,int,char**,char**),
                     void* data);
    private:
        sqlite3* db = nullptr;
    };
}
