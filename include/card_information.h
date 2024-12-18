#ifndef CARD_INFORMATION_H
#define CARD_INFORMATION_H

#include <ryml.hpp>
#include <ryml_std.hpp>
#include <string>
#include <vector>
#include "../include/configuration.h"

void readNode(std::vector<std::string>& information,
              const ryml::Tree& card,
              const c4::yml::ConstNodeRef& configNode,
              const Configuration& configuration);

void appendKeyVal(std::vector<std::string>& information,
                  const c4::yml::ConstNodeRef& keyValNode,
                  std::string& info,
                  const Configuration& configuration,
                  const int& depth);

void fitValue(std::string& value, const unsigned int& terminalWidth, const int& keyWidth);

inline void addLineBreaks(std::string& value, size_t& start, const size_t& end, const unsigned int& maxStringLength);

void appendSequence(std::vector<std::string>& information,
                    const c4::yml::ConstNodeRef& seqNode,
                    const c4::yml::ConstNodeRef& configNode,
                    std::string& info,
                    const Configuration& configuration,
                    int& depth);

void appendMap(std::vector<std::string>& information,
               const c4::yml::ConstNodeRef& mapNode,
               const c4::yml::ConstNodeRef& configNode,
               const std::string& info,
               const Configuration& configuration);

std::string cleanKey(const c4::csubstr& keyCsubstr, const Configuration& configuration);

std::string cleanValue(std::string& valStr, const Configuration& configuration);

inline bool nodeExists(const c4::yml::ConstNodeRef& card, const c4::csubstr& key);

inline void addColorToText(std::string& text, const std::string& textColor);

#endif //CARD_INFORMATION_H
