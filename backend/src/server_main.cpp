#include "httplib.h"
#include "json.hpp"
#include <exception>   // std::exception_ptr, std::rethrow_exception
#include <iostream>
#include <stdexcept>   // std::runtime_error

using json = nlohmann::json;

int main() {
    httplib::Server srv;

    // --- Routes ---
    srv.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        json body;
        body["status"] = "ok";
        res.set_content(body.dump(), "application/json");
    });

  
    // --- Δίχτυ 1: κάθε λάθος χωρίς body (π.χ. 404) -> JSON ---
    srv.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        if (res.body.empty()) {
            json err;
            err["error"] = httplib::status_message(res.status);
            err["path"] = req.path;
            res.set_content(err.dump(), "application/json");
        }
    });

    // --- Δίχτυ 2: exception που δεν έπιασε κανείς -> 500, χωρίς λεπτομέρειες στον client ---
    srv.set_exception_handler([](const httplib::Request&, httplib::Response& res,
                                 std::exception_ptr ep) {
        try {
            std::rethrow_exception(ep);
        } catch (const std::exception& e) {
            std::cerr << "unhandled exception: " << e.what() << std::endl;   // μόνο για σένα
        } catch (...) {
            std::cerr << "unhandled unknown exception" << std::endl;
        }
        res.status = 500;
        json err;
        err["error"] = "internal server error";                             // μόνο αυτό στον client
        res.set_content(err.dump(), "application/json");
    });

    // --- Εκκίνηση ---
    std::cout << "Listening on http://127.0.0.1:8080" << std::endl;
    if (!srv.listen("127.0.0.1", 8080)) {
        std::cerr << "cannot listen on 127.0.0.1:8080 (port already in use?)" << std::endl;
        return 1;
    }
    return 0;
}