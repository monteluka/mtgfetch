#ifndef MANA_SYMBOL_H
#define MANA_SYMBOL_H

#include <string>
#include <vector>
#include "../lib/json.hpp"

std::vector<std::string> getColorIdentity(const nlohmann::json& card);

bool loadManaSymbol(std::vector<std::string>& manaSymbol, const nlohmann::json& card);

bool singleSymbolPrint(std::vector<std::string>& manaSymbol,
                       const std::vector<std::ifstream>& files,
                       std::vector<std::ifstream>::iterator& currentFile);

bool doubleSymbolPrint(std::vector<std::string>& manaSymbol,
                       const std::vector<std::ifstream>& files,
                       std::vector<std::ifstream>::iterator& currentFile);

#endif //MANA_SYMBOL_H
