#include "../include/card_information.h"
#include <iostream>

bool loadInfo(std::vector<std::string>& information, const ryml::Tree& card, const c4::yml::ConstNodeRef& configNode)
{
    static int depth {};
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
        std::cout << configNode << std::endl;
        std::cout << configNode.num_children() << std::endl;
        std::cout << configNode.is_seq() << std::endl;
        std::cout << configNode.is_map() << std::endl;
        std::cout << "node has no value or key" << std::endl;
        --depth;
        for (const c4::yml::ConstNodeRef child : configNode)
        {
            loadInfo(information, card, child);
        }
        return false;
    }

    std::cout << card << std::endl;
    std::cout << card.num_children(card.root_id()) << std::endl;

    const c4::yml::ConstNodeRef el = card[key];

    if (depth < 2)
    {
        info += std::string(key.str, key.len) + ": ";
    }
    else
    {
        info += std::string(2 * depth, ' ') + std::string(key.str, key.len) + ": ";
    }
    if (el.is_keyval()) { appendKeyVal(information, el, info); }
    else if (el.is_seq()) { appendSequence(information, el, configNode, info); }
    else if (el.is_map()) { appendMap(information, el, configNode, info); }
    else {}
    --depth;

    return true;
}

void appendKeyVal(std::vector<std::string>& information,
                  const c4::yml::ConstNodeRef& keyValNode,
                  const std::string& info)
{
    size_t beginning {}, position {}, count {};
    std::string value {std::string(keyValNode.val().str, keyValNode.val().len)};
    if (value.empty() || value == "null") return;
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

void appendSequence(std::vector<std::string>& information,
                    const c4::yml::ConstNodeRef& seqNode,
                    const c4::yml::ConstNodeRef& configNode,
                    std::string& info)
{
    if (seqNode.num_children() == 0)
    {
        return;
    }

    if (seqNode.first_child().is_val())
    {
        for (const auto& element : seqNode.children())
        {
            info += std::string(element.val().str, element.val().len) + ", ";
        }
        info.pop_back();
        info.pop_back();
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
                c4::yml::Tree new_tree {};
                c4::yml::NodeRef parentNode {new_tree.rootref()};
                parentNode |= c4::yml::MAP;

                std::string parentNodeKey {std::string(seqNode.key().str, seqNode.key().len)};
                parentNodeKey = parentNodeKey.substr(parentNodeKey.find('_') + 1);
                parentNodeKey[0] = static_cast<char>(std::toupper(parentNodeKey[0]));
                parentNodeKey.pop_back();
                parentNodeKey += " " + std::to_string(++partNumber);
                info.clear();
                info += std::string(2, ' ') + parentNodeKey;
                information.push_back(info);

                std::cout << configNode << std::endl;

                for (const auto& configNodeChild : configNode.children())
                {
                    c4::csubstr key {};
                    std::cout << configNodeChild << std::endl;
                    std::cout << configNodeChild.has_val() << " " << configNodeChild.has_key() << std::endl;
                    if (configNodeChild.has_val())
                    {
                        key = configNodeChild.val();
                    }
                    else
                    {
                        key = configNodeChild[0].key();
                    }
                    std::cout << key << std::endl;
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
                    loadInfo(information, new_tree, key);
                }
            }
        }
    }
}

void appendMap(std::vector<std::string>& information,
               const c4::yml::ConstNodeRef& mapNode,
               const c4::yml::ConstNodeRef& configNode,
               std::string& info)
{
    if (mapNode.num_children() > 0)
    {
        information.push_back(info);
        c4::yml::Tree new_tree {};
        c4::yml::NodeRef treeRoot {new_tree.rootref()};
        treeRoot |= c4::yml::MAP;

        for (const auto& configNodeChild : configNode.children())
        {
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
            loadInfo(information, new_tree, key);
        }
    }
}
