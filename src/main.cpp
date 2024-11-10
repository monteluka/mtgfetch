#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <filesystem>
#include <httplib.h>
#include <iostream>
#include <ryml.hpp>
// needed since we parse information from std::string
#include <ryml_std.hpp>

#include "../include/card_information.h"
#include "../include/configuration.h"
#include "../include/mana_symbol.h"

void help()
{
    std::cout << "Usage: mtgfetch CARD-NAME" << '\n';
    std::cout << "Usage: mtgfetch OPTION" << '\n';
    std::cout << "mtgfetch fetches information on a single Magic: The Gathering card and displays it to terminal" << '\n';
    std::cout << '\n';
    std::cout << "  -h, --help              shows this message and exit" << '\n';
    std::cout << "  -v, --version           shows version information and exits" << '\n';
    std::cout << "  -r, --random            search for a random card" << '\n';
    std::cout << "      --gen-config        generates a default configuration with popular modules enabled" << '\n';
    std::cout << "      --gen-config-all    generates a configuration file with all modules enables" << std::endl;
}

inline void version()
{
    std::cout << "mtgfetch " << PROJECT_VERSION << std::endl;
}

void generateConfig(const std::filesystem::path& src, const std::filesystem::path& destination)
{
    // check if src exists
    if (!std::filesystem::exists(src))
        throw std::runtime_error(
            std::string("Error: Could not find preset files\nSearched at ") + std::string(src));

    // check if a config file already exists
    if (std::filesystem::exists(destination))
    {
        throw std::runtime_error("Error: Config file already exists.\nCannot generate new config.");
    }
    else
    {
        // config file location does not exist so make it
        std::filesystem::create_directory(destination.parent_path());
        std::filesystem::permissions(destination.parent_path(),
                                     std::filesystem::perms::owner_all | std::filesystem::perms::group_read |
                                     std::filesystem::perms::group_exec | std::filesystem::perms::others_read |
                                     std::filesystem::perms::others_exec, std::filesystem::perm_options::add);
    }

    // copy the file from presets location to created directory
    if (std::filesystem::copy_file(src, destination))
    {
        std::cout << "File successfully created at " << destination << std::endl;
    }
    else { throw std::runtime_error("Error: Could not create config file."); }
}

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

inline bool compareStringIgnCase(const char* const key, const char* const option)
{
    return strcasecmp(key, option) == 0;
}

int parseArguments(const int& argc, char* argv[], std::string& searchType)
{
    if (argc < 2) throw std::runtime_error("Error: No card name entered");
    int positionOfCardName {0};

    // since the only options right now exit after being run, only the first item passed in is checked
    if (const char* currentArg {argv[1]}; currentArg[0] == '-')
    {
        if (compareStringIgnCase(currentArg, "-h") || compareStringIgnCase(currentArg, "--help"))
        {
            help();
        }
        else if (compareStringIgnCase(currentArg, "-v") || compareStringIgnCase(currentArg, "--version"))
        {
            version();
        }
        else if (compareStringIgnCase(currentArg, "--gen-config"))
        {
            std::filesystem::path source {CONFIG_DIR_PATH};
            source += "/default_config.yaml";
#ifdef IS_DEBUG
            std::filesystem::path destination {CONFIG_DIR_PATH};
            destination += "default_config.yaml";
#else
            std::filesystem::path destination {std::getenv("HOME")};
            destination += "/.config/mtgfetch/config.yaml";
#endif
            generateConfig(source, destination);
        }
        else if (compareStringIgnCase(currentArg, "--gen-config-all"))
        {
            std::filesystem::path source {CONFIG_DIR_PATH};
            source += "/all_modules.yaml";
#ifdef IS_DEBUG
            std::filesystem::path destination {CONFIG_DIR_PATH};
            destination += "all_modules.yaml";
#else
            std::filesystem::path destination {std::getenv("HOME")};
            destination += "/.config/mtgfetch/config.yaml";
#endif
            generateConfig(source, destination);
        }
        else if (compareStringIgnCase(currentArg, "-r") || compareStringIgnCase(currentArg, "--random"))
        {
            searchType = "random";
            positionOfCardName = 2;
        }
        else { throw std::runtime_error("Error: Unknown option: " + std::string(currentArg)); }
    }
    else
    {
        positionOfCardName = 1;
    }

    return positionOfCardName;
}

std::string prepareInput(const int& argc, char* argv[], int& currentElement, const std::string& searchType)
{
    if (currentElement > argc) throw std::runtime_error("Error: No card name entered");

    std::string cleanInput {};

    if (searchType != "random")
    {
        // add args to string
        for (; currentElement < argc; ++currentElement)
        {
            cleanInput += argv[currentElement];
            cleanInput += "%20";
        }
        cleanInput.erase(cleanInput.size() - 3);
    }

    return cleanInput;
}

httplib::Result getResult(const std::string& cardSearchName, const std::string& searchType)
{
    httplib::Client cli {"https://api.scryfall.com"};
    cli.set_ca_cert_path("", "/etc/ssl/certs");
    const httplib::Headers headers {{{"User-Agent, mtgfetch/0.1-a", "Accept, application/json"}}};
    return cli.Get("/cards/" + searchType + cardSearchName, headers);
}

int main(const int argc, char* argv[]) try
{
    // set the search type
    std::string searchType {"named?fuzzy="};

    // parse arguments if any
    int currentElement = parseArguments(argc, argv, searchType);
    if (currentElement == 0) return 0;

    // get card name that user entered from terminal
    std::string cardSearchName {prepareInput(argc, argv, currentElement, searchType)};

    // open config and read contents
    const Configuration configuration;

    // get card details from scryfall api
    const httplib::Result res {getResult(cardSearchName, searchType)};

    // make sure we were able to make a connection to the server
    if (!res) throw std::runtime_error("Error: Could not establish connection with the server for card info");
    // make sure we got a valid response back
    if (res->status != 200)
    {
        if (res->status == 404)
        {
            // remove "%20" from string and re-add space character
            size_t repeatPos = cardSearchName.find("%20");
            while (repeatPos < std::string::npos)
            {
                cardSearchName.erase(repeatPos, 3);
                cardSearchName.insert(repeatPos, " ");
                repeatPos = cardSearchName.find("%20");
            }
            throw std::runtime_error(
                "Error: Could not find information on card with name \"" + cardSearchName +
                "\"\nMake sure card name was entered correctly");
        }
        if (res->status == 429)
        {
            throw std::runtime_error(
                "Error: Too many requests\nPlease wait a few minutes and try again.");
        }
        throw std::runtime_error("Error: Did not get valid info for card from server");
    }

    const c4::yml::Tree card {c4::yml::parse_json_in_arena(c4::to_csubstr(res->body))};

    std::vector<std::string> cardInformation {};
    std::vector<std::string> manaSymbol {};

    loadCardInfo(card, cardInformation, configuration);
    if (!loadManaSymbol(manaSymbol, card, configuration)) throw std::runtime_error("Failed to load mana symbol");

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
catch (std::runtime_error& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
