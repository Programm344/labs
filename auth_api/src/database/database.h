#pragma once

#include <sqllite3.h>
#include <string>
#include <iostream>

class Database{
    sqlite3* db;
    static Database* instance;// singleton -  только одно подключение к бд. 
    Database() : db(nullptr) {} //приватный конструктор для синглтона
public:
        static Database& get_instance() { // проверка на единственное подключение. Без создания объекта static вызывать можно.
        if (!instance){
            instance = new Database();
        }
        return *instance; // возвращаем существующий объект
    }
        bool init(){
            int rc = sqlite3_open(auth_db, &db); // создает файл дб и открывает, если не создан. Записывает указатель на бд.

            if (rc) { // 1 - ошибка.
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return false;
            }
            createTables();
             std::cout << "Database initialized" << std::endl;
            return true;

        }
          sqlite3* getDB() { return db; }
          void close() { if (db) sqlite3_close(db); }
private: 
void createTables(){
    // конст указатель на строку символов
    // R"(...)" - строку можно писать в несколько строк
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

        // revoke - отозвать. Отозванные токены.
         const char* revokedTable = R"( 
            CREATE TABLE IF NOT EXISTS revoked_tokens (
                token_hash TEXT PRIMARY KEY,
                user_id INTEGER NOT NULL,
                revoked_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        )";

       
        char* errMsg = nullptr;
        // sql запросы на создание таблиц.
        int rc = sqlite3_exec(db, usersTable, nullptr, nullptr, &errMsg);
         if (rc != SQLITE_OK) {
            std::cerr << "SQL error (users): " << errMsg << std::endl;
            sqlite3_free(errMsg);  
        }
        int rc = sqlite3_exec(db, tokensTable, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error (active_tokens): " << errMsg << std::endl;
            sqlite3_free(errMsg);  
        }
        int rc = sqlite3_exec(db, revokedTable, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error (revoked_tokens): " << errMsg << std::endl;
            sqlite3_free(errMsg);  
        }
    
}          

};
Database* Database::instance = nullptr; // определение и инициализация статической переменной