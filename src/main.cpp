#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <iostream>
#include "../lib/httplib.h"

int main()
{
    httplib::Client cli("http://cpp-httplib-server.yhirose.repl.co");

    auto res = cli.Get("/hi");
    res->status;
    res->body;

    return 0;
}
