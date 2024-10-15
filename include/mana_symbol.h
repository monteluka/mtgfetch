#ifndef MANA_SYMBOL_H
#define MANA_SYMBOL_H

#include <string>
#include <vector>
#include "../lib/json.hpp"

std::vector<std::string> getColorIdentity(nlohmann::json& card);

bool loadManaSymbol(std::vector<std::string>& manaSymbol, nlohmann::json& card);

#endif //MANA_SYMBOL_H
