#include "../include/card_information.h"
#include <iostream>

bool loadInfo(std::vector<std::string>& information, const ryml::Tree& card, const c4::yml::ConstNodeRef& configNode)
{
    static int depth {};
    ++depth;

    c4::csubstr key {};

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
        std::cout << "node has no value or key" << std::endl;
        return false;
    }

    std::cout << "Number of children: " << card[key].num_children() << std::endl;

    const c4::yml::ConstNodeRef el = card[key];

    std::string info {};
    if (depth < 2)
    {
        info += std::string(key.str, key.len) + ": ";
    }
    else
    {
        info += std::string(2 * depth, ' ') + std::string(key.str, key.len) + ": ";
    }
    if (el.is_keyval()) { appendKeyVal(information, el, info); }
    else if (el.is_seq()) { appendSequence(information, el, info); }
    else if (el.is_map()) { appendMap(information, el, info); }
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
                c4::yml::NodeRef treeRoot {new_tree.rootref()};
                treeRoot |= c4::yml::MAP;

                std::string parentNodeKey {std::string(seqNode.key().str, seqNode.key().len)};
                parentNodeKey = parentNodeKey.substr(parentNodeKey.find('_') + 1);
                parentNodeKey[0] = static_cast<char>(std::toupper(parentNodeKey[0]));
                parentNodeKey.pop_back();
                parentNodeKey += " " + std::to_string(++partNumber);

                c4::yml::NodeRef parentNode {treeRoot[c4::to_csubstr(parentNodeKey)]};
                parentNode |= c4::yml::MAP;

                for (const auto& child : element.children())
                {
                    if (child.is_seq())
                    {
                        if (child.num_children() > 0)
                        {
                            c4::yml::NodeRef newSeqNode = parentNode[child.key()];
                            newSeqNode |= c4::yml::SEQ;
                            for (const auto& seqChildNode : child.children())
                            {
                                if (!seqChildNode.val_is_null()) newSeqNode.append_child() = seqChildNode.val();
                            }
                        }
                    }
                    else if (child.is_map())
                    {
                        if (child.num_children() > 0)
                        {
                            c4::yml::NodeRef mapNode = parentNode[child.key()];
                            mapNode |= c4::yml::MAP;
                            for (const auto& mapChildNode : child.children())
                            {
                                mapNode[mapChildNode.key()] = mapChildNode.val();
                            }
                        }
                    }
                    else
                    {
                        parentNode[child.key()] << child.val();
                    }
                }
                // loadInfo(information, new_tree, TODO);
            }
        }
    }
}

void appendMap(std::vector<std::string>& information, const c4::yml::ConstNodeRef& mapNode, std::string& info)
{
    if (mapNode.num_children() > 0)
    {
        information.push_back(info);
        c4::yml::Tree new_tree {};
        c4::yml::NodeRef treeRoot {new_tree.rootref()};
        treeRoot |= c4::yml::MAP;

        for (const auto& child : mapNode.children())
        {
            if (child.is_seq())
            {
                if (child.num_children() > 0)
                {
                    c4::yml::NodeRef seqNode = treeRoot[child.key()];
                    seqNode |= c4::yml::SEQ;
                    for (const auto& seqChildNode : child.children())
                    {
                        if (!seqChildNode.empty()) seqNode.append_child() = seqChildNode.val();
                    }
                }
            }
            else if (child.is_map())
            {
                if (child.num_children() > 0)
                {
                    c4::yml::NodeRef newMapNode = treeRoot[child.key()];
                    newMapNode |= c4::yml::MAP;
                    for (const auto& mapChildNode : child.children())
                    {
                        newMapNode[mapChildNode.key()] = mapChildNode.val();
                    }
                }
            }
            else
            {
                treeRoot[child.key()] << child.val();
            }
        }
        // loadInfo(information, new_tree, TODO);
    }
}
