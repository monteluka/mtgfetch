#ifndef MANA_SYMBOL_H
#define MANA_SYMBOL_H

#include <ryml.hpp>
#include <ryml_std.hpp>
#include <string>
#include <vector>
#include "../include/configuration.h"

std::vector<std::string> getColorIdentity(const ryml::Tree& card);

void loadManaSymbol(std::vector<std::string>& manaSymbol, const c4::yml::Tree& card, const Configuration& configuration);

void addSingleSymbol(std::vector<std::string>& manaSymbol,
                     const std::vector<std::ifstream>& files,
                     std::vector<std::ifstream>::iterator& currentFile);

void addDoubleSymbol(std::vector<std::string>& manaSymbol,
                     const std::vector<std::ifstream>& files,
                     std::vector<std::ifstream>::iterator& currentFile);

void addAllSymbols(std::vector<std::string>& manaSymbol,
                   const std::vector<std::ifstream>& files,
                   std::vector<std::ifstream>::iterator& firstFile);

#endif //MANA_SYMBOL_H
