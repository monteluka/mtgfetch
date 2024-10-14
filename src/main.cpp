#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <iostream>
#include "../lib/httplib.h"

int main()
{
    httplib::Client cli("https://api.scryfall.com");
    cli.set_ca_cert_path("", "/etc/ssl/certs");

    const httplib::Headers headers = {{"User-Agent, mtgfetch/0.1-a", "Accept, application/json"}};
    auto res = cli.Get("/cards/named?fuzzy=wrathful-raptors", headers);

    std::cout << "HTTP status is: " << res->status;
    std::cout << "\n\n" << res->body;

    return 0;
}