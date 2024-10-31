#include "../include/card_information.h"
#include <iostream>
#include <unordered_set>

bool loadInfo(std::vector<std::string>& information,
              const ryml::Tree& card,
              const c4::yml::ConstNodeRef& configNode,
              const Configuration& configuration)
{
    static int depth {-1};
    ++depth;
    c4::csubstr key {};
    std::string info {};

    if (configNode.has_key())
    {
        key = configNode.key();
    }
    else if (configNode.has_val())
    {
        key = configNode.val();
    }
    else
    {
        --depth;
        for (const c4::yml::ConstNodeRef child : configNode)
        {
            loadInfo(information, card, child, configuration);
        }
        return false;
    }


    if (!nodeExists(card, key))
    {
        --depth;
        return false;
    }


    info += std::string(depth * configuration.getIndentLength(), ' ') + cleanKey(key, configuration);

    if (const c4::yml::ConstNodeRef el = card[key]; el.is_keyval())
    {
        appendKeyVal(information, el, info, configuration, depth);
    }
    else if (el.is_seq()) { appendSequence(information, el, configNode, info, configuration, depth); }
    else if (el.is_map()) { appendMap(information, el, configNode, info, configuration); }
    else {}
    --depth;

    return true;
}

void appendKeyVal(std::vector<std::string>& information,
                  const c4::yml::ConstNodeRef& keyValNode,
                  std::string& info,
                  const Configuration& configuration,
                  const int& depth)
{
    std::string value {cleanValue(keyValNode.val())};
    if (value.empty() || value == "Null") return;

    size_t beginning {}, position {}, count {};
    const int space {(depth * 2) + 2};
    bool firstInstance {true};

    info += ": ";
    for (const char& character : value)
    {
        if (character == '\n')
        {
            if (!firstInstance)
            {
                std::string val {value.substr(beginning, count)};
                if (configuration.getColorEnabledOption()) addColorToText(val, configuration.getValTextColor());
                information.push_back(std::string(keyValNode.key().size() + space, ' ') + val);
            }
            else
            {
                std::string val {value.substr(beginning, count)};
                if (configuration.getColorEnabledOption()) addColorToText(val, configuration.getValTextColor());
                information.push_back(info + val);
                firstInstance = false;
            }
            beginning = position + 1;
            count = -1;
        }
        ++count;
        ++position;
    }
    std::string val {value.substr(beginning, count)};
    if (configuration.getColorEnabledOption()) addColorToText(val, configuration.getValTextColor());
    firstInstance
        ? information.push_back(info + val)
        : information.push_back(std::string(keyValNode.key().size() + space, ' ') + val);
}

void appendSequence(std::vector<std::string>& information,
                    const c4::yml::ConstNodeRef& seqNode,
                    const c4::yml::ConstNodeRef& configNode,
                    std::string& info,
                    const Configuration& configuration,
                    int& depth)
{
    if (seqNode.num_children() == 0)
    {
        return;
    }

    if (seqNode.first_child().is_val())
    {
        info += ": ";
        for (const auto& element : seqNode.children())
        {
            std::string val {cleanValue(element.val()) + ", "};
            if (configuration.getColorEnabledOption()) addColorToText(val, configuration.getValTextColor());
            info += val;
        }
        const size_t pos = info.find_last_of(',');
        info.erase(pos, std::string::npos);
        information.push_back(info);
    }
    else
    {
        information.push_back(info);
        int partNumber {0};
        for (const auto& element : seqNode.children())
        {
            if (element.num_children() > 0)
            {
                ++depth;

                c4::yml::Tree new_tree {};
                c4::yml::NodeRef parentNode {new_tree.rootref()};
                parentNode |= c4::yml::MAP;

                // set the key for map object in sequence
                std::string parentNodeKey {std::string(seqNode.key().str, seqNode.key().len)};
                parentNodeKey = parentNodeKey.substr(parentNodeKey.find('_') + 1);
                parentNodeKey[0] = static_cast<char>(std::toupper(parentNodeKey[0]));
                parentNodeKey.pop_back();
                parentNodeKey += " " + std::to_string(++partNumber);
                if (configuration.getColorEnabledOption())
                    addColorToText(
                        parentNodeKey, configuration.getKeyTextColor());
                info.clear();
                info += std::string(depth * configuration.getIndentLength(), ' ') + parentNodeKey;
                information.push_back(info);


                for (const auto& configNodeChild : configNode.children())
                {
                    c4::csubstr key {};
                    if (configNodeChild.has_val())
                    {
                        key = configNodeChild.val();
                    }
                    else
                    {
                        key = configNodeChild[0].key();
                    }
                    if (!nodeExists(element, key)) continue;
                    if (element[key].is_seq())
                    {
                        if (element[key].num_children() > 0)
                        {
                            c4::yml::NodeRef newSeqNode = parentNode[element[key].key()];
                            newSeqNode |= c4::yml::SEQ;
                            for (const auto& seqChildNode : element[key].children())
                            {
                                if (!seqChildNode.val_is_null()) newSeqNode.append_child() = seqChildNode.val();
                            }
                        }
                    }
                    else if (element[key].is_map())
                    {
                        if (element[key].num_children() > 0)
                        {
                            c4::yml::NodeRef mapNode = parentNode[element[key].key()];
                            mapNode |= c4::yml::MAP;
                            for (const auto& mapChildNode : element[key].children())
                            {
                                mapNode[mapChildNode.key()] = mapChildNode.val();
                            }
                        }
                    }
                    else
                    {
                        parentNode[element[key].key()] << element[key].val();
                    }
                }
                for (const auto& key : configNode)
                {
                    loadInfo(information, new_tree, key, configuration);
                }
                --depth;
            }
        }
    }
}

void appendMap(std::vector<std::string>& information,
               const c4::yml::ConstNodeRef& mapNode,
               const c4::yml::ConstNodeRef& configNode,
               const std::string& info,
               const Configuration& configuration)
{
    if (mapNode.num_children() > 0)
    {
        information.push_back(info);
        c4::yml::Tree new_tree {};
        c4::yml::NodeRef treeRoot {new_tree.rootref()};
        treeRoot |= c4::yml::MAP;

        for (const auto& configNodeChild : configNode.children())
        {
            if (!nodeExists(mapNode, configNodeChild.val())) continue;
            if (mapNode[configNodeChild.val()].is_seq())
            {
                if (mapNode[configNodeChild.val()].num_children() > 0)
                {
                    c4::yml::NodeRef seqNode = treeRoot[mapNode[configNodeChild.val()].key()];
                    seqNode |= c4::yml::SEQ;
                    for (const auto& seqChildNode : mapNode[configNodeChild.val()].children())
                    {
                        if (!seqChildNode.empty()) seqNode.append_child() = seqChildNode.val();
                    }
                }
            }
            else if (mapNode[configNodeChild.val()].is_map())
            {
                if (mapNode[configNodeChild.val()].num_children() > 0)
                {
                    c4::yml::NodeRef newMapNode = treeRoot[mapNode[configNodeChild.val()].key()];
                    newMapNode |= c4::yml::MAP;
                    for (const auto& mapChildNode : mapNode[configNodeChild.val()].children())
                    {
                        newMapNode[mapChildNode.key()] = mapChildNode.val();
                    }
                }
            }
            else
            {
                treeRoot[mapNode[configNodeChild.val()].key()] << mapNode[configNodeChild.val()].val();
            }
        }
        for (const auto& key : configNode)
        {
            loadInfo(information, new_tree, key, configuration);
        }
    }
}

std::string cleanKey(const c4::csubstr& keyCsubstr, const Configuration& configuration)
{
    std::string key {keyCsubstr.str, keyCsubstr.len};
    std::unordered_set<std::string> keys {"uri", "png", "cmc", "usd"};

    // check for underscores
    for (int i {1}; i < key.size(); ++i)
    {
        if (key[i] == '_')
        {
            key[i] = ' ';
            if (const int nextLetter {++i}; nextLetter != key.size())
            {
                // if the next three letters are in keys then capitalize them
                if (keys.find(key.substr(nextLetter, 3)) != keys.end())
                {
                    key[nextLetter] = static_cast<char>(std::toupper(key[nextLetter]));
                    key[nextLetter + 1] = static_cast<char>(std::toupper(key[nextLetter + 1]));
                    key[nextLetter + 2] = static_cast<char>(std::toupper(key[nextLetter + 2]));
                }
                // check to see if the next combination is "ID"
                else if (key.substr(nextLetter, 2) == "id")
                {
                    key.replace(nextLetter, 2, "ID");
                }
                // 3 letter combination & "ID" not found so capitalize only first letter after ' '
                else
                {
                    key[nextLetter] = static_cast<char>(std::toupper(key[nextLetter]));
                }
            }
        }
    }

    // check for certain substring if key string is only 2 letters long
    if (key.size() == 2 && key == "id")
    {
        key = "ID";
    }

    // if first 3 letters of the key is in keys set then uppercase them
    if (keys.find(key.substr(0, 3)) != keys.end())
    {
        key[0] = static_cast<char>(std::toupper(key[0]));
        key[1] = static_cast<char>(std::toupper(key[1]));
        key[2] = static_cast<char>(std::toupper(key[2]));
    }

    // capitalize first letter
    key[0] = static_cast<char>(std::toupper(key[0]));

    // apply color if necessary
    if (configuration.getColorEnabledOption()) addColorToText(key, configuration.getKeyTextColor());

    return key;
}

std::string cleanValue(const c4::csubstr& valCsubstr)
{
    std::string val {valCsubstr.str, valCsubstr.len};

    if (const size_t spacePos {val.find(' ')}; spacePos == std::string::npos && val.substr(0, 4) != "http")
    {
        val[0] = static_cast<char>(std::toupper(val[0]));
        if (const size_t pos {val.find('_')}; pos != std::string::npos)
        {
            val[pos] = ' ';
            if (const size_t newPos {pos + 1};
                newPos < val.size()) val[newPos] = static_cast<char>(std::toupper(val[newPos]));
        }
    }

    return val;
}

inline bool nodeExists(const c4::yml::ConstNodeRef& card, const c4::csubstr& key)
{
    return card.find_child(key).id() != c4::yml::NONE;
}

inline void addColorToText(std::string& text, const std::string& textColor)
{
    const std::string resetCode {"\033[0m"};

    text.insert(0, textColor);
    text.append(resetCode);
}
