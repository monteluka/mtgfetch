#ifndef CARD_INFORMATION_H
#define CARD_INFORMATION_H
#include <vector>
#include <string>
#include "../lib/rapidyaml-0.7.2.hpp"
#include "../include/configuration.h"

bool loadInfo(std::vector<std::string>& information,
              const ryml::Tree& card,
              const c4::yml::ConstNodeRef& configNode,
              const Configuration& configuration);

void appendKeyVal(std::vector<std::string>& information,
                  const c4::yml::ConstNodeRef& keyValNode,
                  std::string& info,
                  const Configuration& configuration);

void appendSequence(std::vector<std::string>& information,
                    const c4::yml::ConstNodeRef& seqNode,
                    const c4::yml::ConstNodeRef& configNode,
                    std::string& info,
                    const Configuration& configuration);

void appendMap(std::vector<std::string>& information,
               const c4::yml::ConstNodeRef& mapNode,
               const c4::yml::ConstNodeRef& configNode,
               const std::string& info,
               const Configuration& configuration);

std::string cleanKey(const c4::csubstr& keyCsubstr, const Configuration& configuration);

std::string cleanValue(const c4::csubstr& valCsubstr);

inline bool nodeExists(const c4::yml::ConstNodeRef& card, const c4::csubstr& key);

inline void addColorToText(std::string& text, const std::string& textColor);

#endif //CARD_INFORMATION_H
