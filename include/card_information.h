#ifndef CARD_INFORMATION_H
#define CARD_INFORMATION_H
#include <vector>
#include <string>
#include "../lib/rapidyaml-0.7.2.hpp"

bool loadInfo(std::vector<std::string>& information, const ryml::Tree& card);


#endif //CARD_INFORMATION_H
