#pragma once
#include <sqlite3.h>
#include <string>
#include "../interfaces/i_database_info.h"

namespace services {

class SqliteDatabaseInfo : public interfaces::IDatabaseInfo {
public:
    SqliteDatabaseInfo() {
      
        int rc = sqlite3_open("auth.db", &db_);
        if (rc != SQLITE_OK) {
            connected_ = false;
            error_msg_ = sqlite3_errmsg(db_);
        } else {
            connected_ = true;
            error_msg_ = "";
        }
    }
    
    ~SqliteDatabaseInfo() {
        if (db_) {
            sqlite3_close(db_);
        }
    }
    
    std::string getDriver() const override {
        return "SQLite";
    }
    
    std::string getVersion() const override {
        if (!connected_) return "unknown";
       
        return sqlite3_libversion();
    }
    
    std::string getDatabaseName() const override {
        if (!connected_) return "not connected";
        return "auth.db";  
    }
    
    std::string getStatus() const override {
        if (connected_) {
            return "Connected";
        } else {
            return "Error: " + error_msg_;
        }
    }
    
private:
    sqlite3* db_ = nullptr;
    bool connected_ = false;
    std::string error_msg_;
};

} 