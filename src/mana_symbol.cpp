#include "../include/mana_symbol.h"
#include <fstream>
#include <unordered_map>
#include <utility>

std::vector<std::string> getColorIdentity(nlohmann::json& card)
{
    std::vector<std::string> colorIdentity {};
    for (const auto& color : card["color_identity"]) colorIdentity.push_back(color);
    return colorIdentity;
}

bool loadManaSymbol(std::vector<std::string>& manaSymbol, nlohmann::json& card)
{
    std::vector<std::string> colorIdentity {getColorIdentity(card)};

    const std::unordered_map<char, std::string> colorFile {
        {'U', "blue"},
        {'B', "black"},
        {'G', "green"},
        {'R', "red"},
        {'W', "white"},
        {'C', "colorless"}
    };

    // choose regular or small symbol based on how many colors in identity
    const size_t size {colorIdentity.size()};
    std::string extension {(size > 1) ? "_small.txt" : ".txt"};

    // make a list of open files that are needed
    std::vector<std::ifstream> files {};
    for (const std::string& color : colorIdentity)
    {
        if (auto it = colorFile.find(color[0]); it != colorFile.end())
        {
            std::string filename {"../images/ascii/" + it->second + extension};
            std::ifstream asciiArt {filename, std::ifstream::in};
            if (asciiArt.is_open()) files.emplace_back(std::move(asciiArt));
        }
        else
        {
            return false;
        }
    }

    // load the symbol from the first file into manaSymbol buffer
    // for testing
    for (std::string line; std::getline(files[0], line);)
    {
        manaSymbol.push_back(line);
    }

    return true;
}
