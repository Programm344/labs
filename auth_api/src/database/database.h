#pragma once

#include <sqlite3.h>
#include <string>
#include <iostream>

class Database {
    sqlite3* db;
    static Database* instance;
    
    Database();
    
public:
    static Database& get_instance();
    
    bool init();
    sqlite3* getDB();
    void close();
    
private:
    void createTables();
};