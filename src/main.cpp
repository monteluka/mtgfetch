#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <iostream>
#include "../lib/httplib.h"
#include "../lib/json.hpp"
#include "../include/card_information.h"
#include "../include/mana_symbol.h"

int main()
{
    httplib::Client cli("https://api.scryfall.com");
    cli.set_ca_cert_path("", "/etc/ssl/certs");

    const httplib::Headers headers = {{"User-Agent, mtgfetch/0.1-a", "Accept, application/json"}};
    auto res = cli.Get("/cards/named?fuzzy=kogla-the-titan-ape", headers);

    std::cout << "HTTP status is: " << res->status << '\n';

    nlohmann::json card = nlohmann::json::parse(res->body);

    std::vector<std::string> cardInformation {};

    loadInfo(cardInformation, card);

    for (const auto& sentence : cardInformation)
        std::cout << sentence << std::endl;

    std::cout << "\n\n\n";

    for (auto& el : card.items()) {
        std::cout << el.key() << " : " << el.value() << "\n";
    }

    std::cout << "\n\n\n";

    std::cout << std::setw(4) << card << std::endl;

    return 0;
}