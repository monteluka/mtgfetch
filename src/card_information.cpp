#include "../include/card_information.h"

bool loadInfo(std::vector<std::string>& information, nlohmann::json& card)
{
    information.push_back("Name: " + std::string {card["name"]});
    information.push_back("Mana Cost: " + std::string {card["mana_cost"]});
    information.push_back("Type: " + std::string {card["type_line"]});
    information.push_back("Description: " + std::string {card["oracle_text"]});
    return true;
}
