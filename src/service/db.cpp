#include "db.h"
#include <stdexcept>

using namespace jetfire27::Engine::DB;

SQLiteDB::SQLiteDB(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK)
        throw std::runtime_error("Cannot open or create DB");
}

SQLiteDB::~SQLiteDB() {
    if (db) sqlite3_close(db);
}

void SQLiteDB::Execute(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string e{err};
        sqlite3_free(err);
        throw std::runtime_error(e);
    }
}

void SQLiteDB::Execute(const std::string& sql,
                       int (*callback)(void*,int,char**,char**),
                       void* data)
{
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), callback, data, &err) != SQLITE_OK) {
        std::string e{err};
        sqlite3_free(err);
        throw std::runtime_error("SQL error: " + e);
    }
}
