#include "httplib.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

int main() {
    httplib::Server srv;
    std::cout<<"Listening on http://127.0.0.1:8080"<<std::endl;
    srv.Get("/api/health",[](const httplib::Request&,httplib::Response& res){
        json body;
        body["status"]="ok";
        res.set_content(body.dump(),"application/json");
    });
    if(!srv.listen("127.0.0.1",8080)){
        std::cerr<<"cannot listen on 127.0.0.1:8080 (port already in use?"<<std::endl;
        return 1;
    };

    return 0;
}