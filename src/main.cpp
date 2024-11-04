#define CPPHTTPLIB_OPENSSL_SUPPORT
#define RYML_SINGLE_HDR_DEFINE_NOW

#include <iostream>
#include "../lib/httplib.h"
#include "../lib/rapidyaml-0.7.2.hpp"
#include "../include/card_information.h"
#include "../include/configuration.h"
#include "../include/mana_symbol.h"

void loadCardInfo(const c4::yml::Tree& card,
                  std::vector<std::string>& cardInformation,
                  const Configuration& configuration)
{
    const c4::yml::Tree& config {configuration.getConfigTree()};
    // loop through each key in config
    for (const c4::yml::ConstNodeRef module : config["modules"])
    {
        // check if the key in the config exists in card keys
        if (module.has_val() && card.find_child(card.root_id(), module.val()) != c4::yml::NONE)
        {
            readNode(cardInformation, card, module, configuration);
        }
        else if (module.has_children() && module.first_child().has_key() && card.find_child(
                     card.root_id(), module.first_child().key()) !=
                 c4::yml::NONE)
        {
            readNode(cardInformation, card, module.first_child(), configuration);
        }
        else {}
    }
}

std::string prepareInput(const int& argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "No name of card entered!" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::string cleanInput {};

    // add args to string
    for (int i {1}; i < argc; ++i)
    {
        cleanInput += argv[i];
        cleanInput += "%20";
    }
    cleanInput.erase(cleanInput.size() - 3, 3);
    return cleanInput;
}

httplib::Result getResult(const std::string& cardSearchName)
{
    httplib::Client cli {"https://api.scryfall.com"};
    cli.set_ca_cert_path("", "/etc/ssl/certs");
    const httplib::Headers headers {{{"User-Agent, mtgfetch/0.1-a", "Accept, application/json"}}};
    return cli.Get("/cards/named?fuzzy=" + cardSearchName, headers);
}

int main(int argc, char* argv[])
{
    // get card name that user entered from terminal
    const std::string cardSearchName {prepareInput(argc, argv)};
    // open config and read contents
    const Configuration configuration;
    // get card details from scryfall api
    const httplib::Result res {getResult(cardSearchName)};

    const c4::yml::Tree card {c4::yml::parse_json_in_arena(c4::to_csubstr(res->body))};

    std::vector<std::string> cardInformation {};
    std::vector<std::string> manaSymbol {};

    loadCardInfo(card, cardInformation, configuration);
    if (!loadManaSymbol(manaSymbol, card, configuration))
    {
        std::cerr << "ERROR LOADING MANA SYMBOL" << std::endl;
        return -1;
    }

    const size_t largestBuffer {std::max(cardInformation.size(), manaSymbol.size())};
    const int fillerSpace {manaSymbol.size() == 19 ? 39 : 43};
    for (size_t i = 0; i < largestBuffer; i++)
    {
        if (i < manaSymbol.size())
        {
            std::cout << manaSymbol[i] << "   ";
        }
        else
        {
            std::cout << std::string(fillerSpace, ' ');
        }

        if (i < cardInformation.size())
        {
            std::cout << cardInformation[i];
        }

        std::cout << '\n';
    }

    return 0;
}
