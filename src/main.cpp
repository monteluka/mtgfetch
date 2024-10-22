#define CPPHTTPLIB_OPENSSL_SUPPORT
#define RYML_SINGLE_HDR_DEFINE_NOW

#include <iostream>
#include "../lib/httplib.h"
#include "../lib/rapidyaml-0.7.2.hpp"
#include "../include/card_information.h"
#include "../include/mana_symbol.h"

int main()
{
    httplib::Client cli {"https://api.scryfall.com"};
    cli.set_ca_cert_path("", "/etc/ssl/certs");

    const httplib::Headers headers {{{"User-Agent, mtgfetch/0.1-a", "Accept, application/json"}}};
    auto res {cli.Get("/cards/named?fuzzy=colossol-sky-turtle", headers)};

    std::cout << "HTTP status is: " << res->status << '\n';

    c4::yml::Tree card {c4::yml::parse_json_in_arena(c4::to_csubstr(res->body))};

    std::vector<std::string> cardInformation {};
    std::vector<std::string> manaSymbol {};

    loadInfo(cardInformation, card);
    if (!loadManaSymbol(manaSymbol, card))
    {
        std::cout << "ERROR LOADING MANA SYMBOL" << std::endl;
        return -1;
    }

    const size_t largestBuffer {std::max(cardInformation.size(), manaSymbol.size())};

    for (size_t i = 0; i < largestBuffer; i++)
    {
        if (i < manaSymbol.size())
        {
            std::cout << manaSymbol[i] << "   ";
        }
        else
        {
            std::cout << ((manaSymbol[0].size() == 40) ? std::string(43, ' ') : std::string(39, ' '));
        }

        if (i < cardInformation.size())
        {
            std::cout << cardInformation[i];
        }

        std::cout << '\n';
    }


    // std::cout << "\n\n\n";

    // for (auto& el : card.items())
    // {
    // std::cout << el.key() << " : " << el.value() << "\n";
    // }

    // std::cout << "\n\n\n";

    // std::cout << std::setw(4) << card << std::endl;

    return 0;
}
