#include "../include/mana_symbol.h"
#include <fstream>
#include <unordered_map>
#include <utility>

std::vector<std::string> getColorIdentity(const ryml::Tree& card)
{
    std::vector<std::string> colorIdentity {};
    if (card["color_identity"].num_children() == 0) colorIdentity.emplace_back("C");
    else for (const auto& color : card["color_identity"]) colorIdentity.emplace_back(color.val().str);
    return colorIdentity;
}

void loadManaSymbol(std::vector<std::string>& manaSymbol, const c4::yml::Tree& card, const Configuration& configuration)
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
    std::string extension {};
    if (configuration.getColorEnabledOption())
    {
        extension = (size > 1) ? "_small_colored.txt" : "_colored.txt";
    }
    else
    {
        extension = (size > 1) ? "_small.txt" : ".txt";
    }

    // make a list of open files that are needed
    std::vector<std::ifstream> files {};
    for (const std::string& color : colorIdentity)
    {
        if (auto it = colorFile.find(color[0]); it != colorFile.end())
        {
            std::string filename {ASCII_ART_PATH + it->second + extension};
            std::ifstream asciiArt {filename, std::ifstream::in};
            if (asciiArt.is_open()) files.emplace_back(std::move(asciiArt));
            asciiArt.close();
        }
    }

    std::vector<std::ifstream>::iterator firstFile {files.begin()};
    addAllSymbols(manaSymbol, files, firstFile);
}

void addSingleSymbol(std::vector<std::string>& manaSymbol,
                     const std::vector<std::ifstream>& files,
                     std::vector<std::ifstream>::iterator& currentFile)
{
    if (currentFile == files.end())
    {
        // nothing to add
        return;
    }

    if (currentFile != files.begin())
    {
        for (std::string line {}; std::getline(*currentFile, line);)
        {
            manaSymbol.push_back(std::string(10, ' ') + line + std::string(10, ' '));
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
}

void addDoubleSymbol(std::vector<std::string>& manaSymbol,
                     const std::vector<std::ifstream>& files,
                     std::vector<std::ifstream>::iterator& currentFile)
{
    const std::vector<std::ifstream>::iterator nextFile {std::next(currentFile)};
    if (currentFile == files.end() || nextFile == files.end())
    {
        // nothing to add
        return;
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
}

void addAllSymbols(std::vector<std::string>& manaSymbol,
                   const std::vector<std::ifstream>& files,
                   std::vector<std::ifstream>::iterator& firstFile)
{
    const int numberOfSymbols {static_cast<int>(files.size())};

    // there are no symbols to display
    if (numberOfSymbols == 0) return;

    // calculate the number of double pritns and single prints we need
    const int numberOfDoublePrints {numberOfSymbols / 2};
    const int numberOfSinglePrints {numberOfSymbols % 2};

    // add as many double symbols and single symbols that we need
    for (int i {0}; i < numberOfDoublePrints; i++) addDoubleSymbol(manaSymbol, files, firstFile);
    for (int i {0}; i < numberOfSinglePrints; i++) addSingleSymbol(manaSymbol, files, firstFile);
}
