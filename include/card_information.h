#ifndef CARD_INFORMATION_H
#define CARD_INFORMATION_H
#include <vector>
#include <string>
#include "../lib/rapidyaml-0.7.2.hpp"

bool loadInfo(std::vector<std::string>& information, const ryml::Tree& card, const c4::yml::ConstNodeRef& configNode);

void appendKeyVal(std::vector<std::string>& information, const c4::yml::ConstNodeRef& keyValNode, const std::string& info);
void appendSequence(std::vector<std::string>& information, const c4::yml::ConstNodeRef& seqNode, const c4::yml::ConstNodeRef& configNode, std::string& info);
void appendMap(std::vector<std::string>& information, const c4::yml::ConstNodeRef& mapNode, const c4::yml::ConstNodeRef& configNode, std::string& info);

std::string titleCase(c4::csubstr keyCsubstr);

#endif //CARD_INFORMATION_H
