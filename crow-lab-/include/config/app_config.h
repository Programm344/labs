#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <string>

namespace config {
const std::string LOCALE = "ru";
const std::string TIMEZONE = "Moscow";

// сервер
const int PORT = 8080;
const int THREAD_COUNT = 4;

}
#endif 
