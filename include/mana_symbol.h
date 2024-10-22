#ifndef MANA_SYMBOL_H
#define MANA_SYMBOL_H

#include <string>
#include <vector>
#include "../lib/rapidyaml-0.7.2.hpp"

std::vector<std::string> getColorIdentity(const ryml::Tree& card);

bool loadManaSymbol(std::vector<std::string>& manaSymbol, const c4::yml::Tree& card);

bool addSingleSymbol(std::vector<std::string>& manaSymbol,
                     const std::vector<std::ifstream>& files,
                     std::vector<std::ifstream>::iterator& currentFile);

bool addDoubleSymbol(std::vector<std::string>& manaSymbol,
                     const std::vector<std::ifstream>& files,
                     std::vector<std::ifstream>::iterator& currentFile);

bool addAllSymbols(std::vector<std::string>& manaSymbol,
                   const std::vector<std::ifstream>& files,
                   std::vector<std::ifstream>::iterator& firstFile);

#endif //MANA_SYMBOL_H
