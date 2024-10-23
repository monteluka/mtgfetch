#include "../include/card_information.h"
#include <iostream>

bool loadInfo(std::vector<std::string>& information, const ryml::Tree& card)
{
    static int depth {};
    ++depth;

    c4::yml::ConstNodeRef root {card.crootref()};

    for (const auto& el : root.children())
    {
        std::string info {};
        if (depth < 2)
        {
            info += std::string(el.key().str, el.key().len) + ": ";
        }
        else
        {
            info += std::string(2 * depth, ' ') + std::string(el.key().str, el.key().len) + ": ";
        }
        if (el.is_keyval())
        {
            size_t beginning {}, position {}, count {};
            std::string value {std::string(el.val().str, el.val().len)};
            if (value.empty() || value == "null") continue;
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
        else if (el.is_seq())
        {
            if (el.num_children() == 0)
            {
                continue;
            }

            if (el.first_child().is_val())
            {
                for (const auto& element : el.children())
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
                for (const auto& element : el.children())
                {
                    if (element.num_children() > 0)
                    {
                        c4::yml::Tree new_tree {};
                        c4::yml::NodeRef treeRoot {new_tree.rootref()};
                        treeRoot |= c4::yml::MAP;

                        std::string parentNodeKey {std::string(el.key().str, el.key().len)};
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
                                    c4::yml::NodeRef seqNode = parentNode[child.key()];
                                    seqNode |= c4::yml::SEQ;
                                    for (const auto& seqChildNode : child.children())
                                    {
                                        if (!seqChildNode.val_is_null()) seqNode.append_child() = seqChildNode.val();
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
                        loadInfo(information, new_tree);
                    }
                }
            }
        }
        else if (el.is_map())
        {
            if (el.num_children() > 0)
            {
                information.push_back(info);
                c4::yml::Tree new_tree {};
                c4::yml::NodeRef treeRoot {new_tree.rootref()};
                treeRoot |= c4::yml::MAP;

                for (const auto& child : el.children())
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
                            c4::yml::NodeRef mapNode = treeRoot[child.key()];
                            mapNode |= c4::yml::MAP;
                            for (const auto& mapChildNode : child.children())
                            {
                                mapNode[mapChildNode.key()] = mapChildNode.val();
                            }
                        }
                    }
                    else
                    {
                        treeRoot[child.key()] << child.val();
                    }
                }
                loadInfo(information, new_tree);
            }
        }
        else {}
    }
    --depth;

    return true;
}
