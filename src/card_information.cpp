#include "../include/card_information.h"
#include <iostream>
#include <sys/stat.h>

bool loadInfo(std::vector<std::string>& information, nlohmann::json& card)
{
    for (auto& el : card.items())
    {
        std::string info {};
        info += el.key() + ": ";
        if (el.value().is_string())
        {
            size_t beginning {}, position {}, count {};
            std::string value {el.value()};
            bool firstInstance {true};
            for (const char& character : value)
            {
                if (character == '\n')
                {
                    if (!firstInstance)
                    {
                        information.push_back(std::string(info.size(), ' ') + value.substr(beginning, count));
                    }
                    else
                    {
                        information.push_back(info + value.substr(beginning, count));
                        firstInstance = false;
                    }
                    beginning = position + 1;
                    count = -1;
                }
                ++count;
                ++position;
            }
            firstInstance
                ? information.push_back(info + value.substr(beginning, count))
                : information.push_back(std::string(info.size(), ' ') + value.substr(beginning, count));
        }
        else if (el.value().is_boolean())
        {
            info += el.value() ? "True" : "False";
            information.push_back(info);
        }
        else if (el.value().is_number())
        {
            info += std::to_string(el.value().get<int>());
            information.push_back(info);
        }
        else if (el.value().is_array())
        {
            if (el.value()[0].is_number())
            {
                for (auto& element : el.value())
                {
                    info += std::to_string(element.get<int>()) + ", ";
                }
            }
            else
            {
                for (auto& element : el.value())
                {
                    info += element;
                    info += ", ";
                }
            }
            info.pop_back();
            info.pop_back();
        }
        else if (el.value().is_object())
        {
            loadInfo(information, el.value());
        }
        else {}
    }

    return true;
}
