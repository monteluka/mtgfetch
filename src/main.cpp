#define CPPHTTPLIB_OPENSSL_SUPPORT
#define RYML_SINGLE_HDR_DEFINE_NOW

#include <iostream>
#include "../lib/httplib.h"
#include "../lib/rapidyaml-0.7.2.hpp"
#include "../include/card_information.h"
#include "../include/configuration.h"
#include "../include/mana_symbol.h"

int main()
{
    httplib::Client cli {"https://api.scryfall.com"};
    cli.set_ca_cert_path("", "/etc/ssl/certs");

    const httplib::Headers headers {{{"User-Agent, mtgfetch/0.1-a", "Accept, application/json"}}};
    auto res {cli.Get("/cards/named?fuzzy=esika-god-of-the-tree", headers)};

    std::cout << "HTTP status is: " << res->status << '\n';

    c4::yml::Tree card {c4::yml::parse_json_in_arena(c4::to_csubstr(res->body))};

    std::vector<std::string> cardInformation {};
    std::vector<std::string> manaSymbol {};

    // scratchpad
    // open config and read contents
    const Configuration configuration;
    const c4::yml::Tree& config {configuration.getConfigTree()};

    std::cout << c4::yml::as_json(card) << "\n\n\n" << config << std::endl;

    // loop through each key in config
    for (const c4::yml::ConstNodeRef module : config["modules"])
    {
        // check if the key in the config exists in card keys
        if (module.has_val() && card.find_child(card.root_id(), module.val()) != c4::yml::NONE)
        {
            std::cout << module.val() << std::endl;
            loadInfo(cardInformation, card, module);
        }
        else if (module.has_children() && module.first_child().has_key() && card.find_child(
                     card.root_id(), module.first_child().key()) !=
                 c4::yml::NONE)
        {
            loadInfo(cardInformation, card, module.first_child());
        }
        else
        {
            std::cout << "I HAVE NOTING" << std::endl;
        }
    }

    // end scratchpad


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
            std::cout << ((manaSymbol[0].size() > 40) ? std::string(43, ' ') : std::string(39, ' '));
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
