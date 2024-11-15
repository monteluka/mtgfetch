#include "../include/card_information.h"
#include <unordered_set>

void readNode(std::vector<std::string>& information,
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
        if (configNode.has_children())
        {
            for (const c4::yml::ConstNodeRef& child : configNode)
            {
                readNode(information, card, child, configuration);
            }
        }
        return;
    }

    // item in config does not exist in card tree
    if (!nodeExists(card, key))
    {
        --depth;
        return;
    }

    info += std::string(depth * configuration.getIndentLength(), ' ') + cleanKey(key, configuration);

    if (const c4::yml::ConstNodeRef el = card[key]; el.is_keyval())
    {
        appendKeyVal(information, el, info, configuration, depth);
    }
    else if (el.is_seq()) { appendSequence(information, el, configNode, info, configuration, depth); }
    else if (el.is_map()) { appendMap(information, el, configNode, info, configuration); }
    // in case a non keyval/seq/map node is found then do nothing
    else {}
    --depth;
}

void appendKeyVal(std::vector<std::string>& information,
                  const c4::yml::ConstNodeRef& keyValNode,
                  std::string& info,
                  const Configuration& configuration,
                  const int& depth)
{
    size_t beginning {}, position {}, count {};
    // the 2 is to account for the ": " that goes after the key in the output
    const int space {static_cast<int>((depth * configuration.getIndentLength()) + keyValNode.key().size() + 2)};
    bool firstInstance {true};

    std::string value {keyValNode.val().str, keyValNode.val().len};
    if (value.empty() || value == "null") return;
    fitValue(value, configuration.getTerminalWidth(), space);
    cleanValue(value, configuration);

    info += ": ";
    for (const char& character : value)
    {
        if (character == '\n')
        {
            if (!firstInstance)
            {
                std::string val {value.substr(beginning, count)};
                information.push_back(std::string(space, ' ') + val);
            }
            else
            {
                std::string val {value.substr(beginning, count)};
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
    firstInstance
        ? information.push_back(info + val)
        : information.push_back(std::string(space, ' ') + val);
}

void fitValue(std::string& value, const unsigned int& terminalWidth, const int& keyWidth)
{
    // the size of two small symbols put together is 40 characters wide and add 3 for spacing in between symbol & text
    // since this is the widest amount of space a symbol can be, we use it for the length
    constexpr int manaSymbolLength {43};
    // don't bother trying to fit anything if terminal is too small or if outputting to file
    if (terminalWidth <= static_cast<unsigned int>(manaSymbolLength + keyWidth)) return;
    const unsigned int maxStringLength {terminalWidth - (manaSymbolLength + keyWidth)};

    const size_t end {value.size()};
    size_t start {0};
    if (value.find('\n') == std::string::npos)
    {
        addLineBreaks(value, start, end, maxStringLength);
    }
    else
    {
        while (start < end)
        {
            size_t substrEnd {value.find('\n', start + 1)};
            if (substrEnd == std::string::npos) substrEnd = end;
            addLineBreaks(value, start, substrEnd, maxStringLength);
            start = substrEnd + 1;
        }
    }
}

inline void addLineBreaks(std::string& value, size_t& start, const size_t& end, const unsigned int& maxStringLength)
{
    // add a newline character at first space character found before maxStringLength
    while (end - start > maxStringLength)
    {
        const size_t breakPos {value.rfind(' ', start + maxStringLength)};
        if (breakPos == std::string::npos) break;
        value.insert(breakPos, "\n");
        if (breakPos + 1 < end) value.erase(breakPos + 1, 1);
        start = breakPos + 1;
    }
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
            std::string val {std::string(element.val().str, element.val().len) + ", "};
            cleanValue(val, configuration);
            info += val;
        }
        const size_t pos = info.rfind(',', info.size());
        info.erase(pos);
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
                        const std::string value {
                            std::string(element[key].val().str,
                                        element[key].val().len)
                        };
                        if (value.empty() || value == "null") continue;
                        parentNode[element[key].key()] << element[key].val();
                    }
                }

                // check to see if new_tree has children (values)
                // if not then we skip so that dangling key isn't added to output
                if (new_tree.has_children(new_tree.root_id()))
                {
                    information.push_back(info);
                    for (const auto& key : configNode)
                    {
                        readNode(information, new_tree, key, configuration);
                    }
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
                const std::string value {
                    std::string(mapNode[configNodeChild.val()].val().str,
                                mapNode[configNodeChild.val()].val().len)
                };
                if (value.empty() || value == "null") continue;
                treeRoot[mapNode[configNodeChild.val()].key()] << value;
            }
        }

        // check to see if new_tree has children (values)
        // if not then we skip so that dangling key isn't added to output
        if (new_tree.has_children(new_tree.root_id()))
        {
            information.push_back(info);
            for (const auto& key : configNode)
            {
                readNode(information, new_tree, key, configuration);
            }
        }
    }
}

std::string cleanKey(const c4::csubstr& keyCsubstr, const Configuration& configuration)
{
    std::string key {keyCsubstr.str, keyCsubstr.len};
    std::unordered_set<std::string> keys {"uri", "png", "cmc", "usd"};

    // check for underscores
    for (size_t i {1}; i < key.size(); ++i)
    {
        if (key[i] == '_')
        {
            key[i] = ' ';
            if (const size_t nextLetter {++i}; nextLetter != key.size())
            {
                // if the next three letters are in keys then capitalize them
                if (keys.find(key.substr(nextLetter, 3)) != keys.end())
                {
                    key[nextLetter] = static_cast<char>(std::toupper(key[nextLetter]));
                    key[nextLetter + 1] = static_cast<char>(std::toupper(key[nextLetter + 1]));
                    key[nextLetter + 2] = static_cast<char>(std::toupper(key[nextLetter + 2]));
                }
                // make sure that we are at the end of the string
                // check to see if the next combination is "ID"
                else if (nextLetter + 2 == key.size() && key.substr(nextLetter, 2) == "id")
                {
                    key[nextLetter] = static_cast<char>(std::toupper(key[nextLetter]));
                    key[nextLetter + 1] = static_cast<char>(std::toupper(key[nextLetter + 1]));
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

std::string cleanValue(std::string& valStr, const Configuration& configuration)
{
    if (const size_t spacePos {valStr.find(' ')}; spacePos == std::string::npos && valStr.substr(0, 4) != "http")
    {
        valStr[0] = static_cast<char>(std::toupper(valStr[0]));
        if (const size_t pos {valStr.find('_')}; pos != std::string::npos)
        {
            valStr[pos] = ' ';
            if (const size_t newPos {pos + 1};
                newPos < valStr.size()) valStr[newPos] = static_cast<char>(std::toupper(valStr[newPos]));
        }
    }

    // this check is for if we are checking elements of a sequence
    if (valStr.size() > 2 && valStr.substr(valStr.size() - 2) == ", ")
    {
        valStr[0] = static_cast<char>(std::toupper(valStr[0]));
    }

    if (configuration.getColorEnabledOption()) addColorToText(valStr, configuration.getValTextColor());

    return valStr;
}

inline bool nodeExists(const c4::yml::ConstNodeRef& card, const c4::csubstr& key)
{
    return card.find_child(key).id() != c4::yml::NONE;
}

inline void addColorToText(std::string& text, const std::string& textColor)
{
    // in case an empty text color was added then return
    if (textColor.empty()) return;

    const std::string resetCode {"\033[0m"};
    text.insert(0, textColor);
    text.append(resetCode);

    if (size_t lineBreakPos {text.find('\n')}; lineBreakPos != std::string::npos)
    {
        while (lineBreakPos != std::string::npos)
        {
            text.insert(lineBreakPos, resetCode);
            lineBreakPos += resetCode.size();
            text.insert(lineBreakPos + 1, textColor);
            lineBreakPos += textColor.size();
            lineBreakPos = text.find('\n', lineBreakPos);
        }
    }
}
