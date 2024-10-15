#ifndef CARD_INFORMATION_H
#define CARD_INFORMATION_H
#include <vector>
#include <string>
#include "../lib/json.hpp"

bool loadInfo(std::vector<std::string>& information, nlohmann::json& card);


#endif //CARD_INFORMATION_H
