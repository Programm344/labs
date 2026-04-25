#include "database.h"

Database* Database::instance = nullptr;

Database::Database() : db(nullptr) {}

Database& Database::get_instance() {
    if (!instance) {
        instance = new Database();
    }
    return *instance;
}

bool Database::init() {
    int rc = sqlite3_open("auth.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    createTables();
    std::cout << "Database initialized" << std::endl;
    return true;
}

sqlite3* Database::getDB() {
    return db;
}

void Database::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

void Database::createTables() {
    const char* usersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            birthday TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    const char* tokensTable = R"(
        CREATE TABLE IF NOT EXISTS active_tokens (
            token_hash TEXT PRIMARY KEY,
            user_id INTEGER NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            expires_at DATETIME NOT NULL,
            is_refresh INTEGER DEFAULT 0
        )
    )";
    
    const char* revokedTable = R"(
        CREATE TABLE IF NOT EXISTS revoked_tokens (
            token_hash TEXT PRIMARY KEY,
            user_id INTEGER NOT NULL,
            revoked_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    char* errMsg = nullptr;
    
    int rc = sqlite3_exec(db, usersTable, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (users): " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    rc = sqlite3_exec(db, tokensTable, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (active_tokens): " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    rc = sqlite3_exec(db, revokedTable, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (revoked_tokens): " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}