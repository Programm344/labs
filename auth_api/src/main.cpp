#include <crow.h>

int main() {
    crow::SimpleApp app;
    
    CROW_ROUTE(app, "/")
    ([](){
        return "Auth API is running!";
    });
    
    app.port(8080).run();
    return 0;
}
