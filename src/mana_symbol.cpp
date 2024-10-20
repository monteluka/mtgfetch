#include "../include/mana_symbol.h"
#include <fstream>
#include <unordered_map>
#include <utility>

std::vector<std::string> getColorIdentity(const nlohmann::json& card)
{
    std::vector<std::string> colorIdentity {};
    if (card["color_identity"][0].is_null()) colorIdentity.emplace_back("C");
    else for (const auto& color : card["color_identity"]) colorIdentity.push_back(color);
    return colorIdentity;
}

bool loadManaSymbol(std::vector<std::string>& manaSymbol, const nlohmann::json& card)
{
    std::vector<std::string> colorIdentity {getColorIdentity(card)};

    const std::unordered_map<char, std::string> colorFile {
        {'U', "blue"},
        {'B', "black"},
        {'G', "green"},
        {'R', "red"},
        {'W', "white"},
        {'C', "colorless"},
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

    if (std::vector<std::ifstream>::iterator firstFile {files.begin()}; !addAllSymbols(manaSymbol, files, firstFile))
    {
        return false;
    }

    return true;
}

bool addSingleSymbol(std::vector<std::string>& manaSymbol,
                     const std::vector<std::ifstream>& files,
                     std::vector<std::ifstream>::iterator& currentFile)
{
    if (currentFile == files.end())
    {
        return false;
    }

    if (currentFile != files.begin())
    {
        for (std::string line {}; std::getline(*currentFile, line);)
        {
            manaSymbol.push_back(std::string(9, ' ') + line + std::string(11, ' '));
        }
    }
    else
    {
        for (std::string line {}; std::getline(*currentFile, line);)
        {
            manaSymbol.push_back(line);
        }
    }

    ++currentFile;
    return true;
}

bool addDoubleSymbol(std::vector<std::string>& manaSymbol,
                     const std::vector<std::ifstream>& files,
                     std::vector<std::ifstream>::iterator& currentFile)
{
    const std::vector<std::ifstream>::iterator nextFile {std::next(currentFile)};
    if (currentFile == files.end() || nextFile == files.end())
    {
        return false;
    }

    std::string manaSymbol1 {};
    std::string manaSymbol2 {};

    while (std::getline(*currentFile, manaSymbol1) && std::getline(*nextFile, manaSymbol2))
    {
        manaSymbol.push_back(manaSymbol1 + manaSymbol2);
    }

    if (std::next(nextFile) != files.end())
    {
        currentFile += 2;
    }
    else
    {
        // remove constness of files vector to assign end of vector to iterator
        // files vector is not being modified at all
        currentFile = const_cast<std::vector<std::ifstream>&>(files).end();
    }

    return true;
}

bool addAllSymbols(std::vector<std::string>& manaSymbol,
                   const std::vector<std::ifstream>& files,
                   std::vector<std::ifstream>::iterator& firstFile)
{
    const int numberOfSymbols {static_cast<int>(files.size())};

    // there are no symbols to display - error
    if (numberOfSymbols == 0)
    {
        return false;
    }

    // there is only one single symbol to display
    if (numberOfSymbols == 1)
    {
        if (!addSingleSymbol(manaSymbol, files, firstFile))
        {
            return false;
        }
        // adding the single symbol was a success
        return true;
    }

    // only get here if we have multiple symbols to add to the manaSymbol vector
    const int numberOfDoublePrints {numberOfSymbols / 2};
    const int numberOfSinglePrints {numberOfSymbols % 2};

    for (int i {0}; i < numberOfDoublePrints; i++)
    {
        if (!addDoubleSymbol(manaSymbol, files, firstFile))
        {
            return false;
        }
    }

    for (int i {0}; i < numberOfSinglePrints; i++)
    {
        if (!addSingleSymbol(manaSymbol, files, firstFile))
        {
            return false;
        }
    }

    return true;
}
