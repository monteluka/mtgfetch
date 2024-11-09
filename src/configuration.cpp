#include "../include/configuration.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/ioctl.h>

Configuration::Configuration()
{
    // get terminal width
    if (winsize size {}; ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0) m_terminalWidth = size.ws_col;

    // find config file
    std::string configFilePath {findConfigFile()};

    // open the file and create tree
    std::string configContents {};
    char character {};
    std::ifstream configFile {configFilePath, std::ifstream::in};
    if (configFile.is_open())
    {
        while (configFile.get(character)) configContents += character;
    }
    else
    {
        throw std::runtime_error("Error: Could not open config file at " + configFilePath);
    }
    configFile.close();

    m_configTree = {c4::yml::parse_in_arena(c4::to_csubstr(configContents))};

    // check if configTree is valid
    if (!m_configTree.is_map(m_configTree.root_id()))
    {
        throw std::runtime_error(
            "Error: No keys in config file\nUnable to parse contents.");
    }
    // check if modules section exists in config file
    if (m_configTree.find_child(m_configTree.root_id(), "modules") == c4::yml::NONE)
    {
        throw std::runtime_error("Error: No \"modules\" section found in config file");
    }
    // make sure modules section has values
    if (!m_configTree["modules"].has_children())
    {
        throw std::runtime_error(
            "Error: \"Modules\" section in config file is empty.\nPlease add card info keys that you want in output.");
    }

    // set configuration class options from config tree
    if (c4::yml::ConstNodeRef configNode = m_configTree.crootref(); !configNode.find_child("options").invalid())
    {
        configNode = configNode["options"];
        if (!configNode.is_map()) return;
        if (!configNode.find_child("color").invalid())
        {
            if (const c4::yml::ConstNodeRef colorNode {configNode["color"]}; colorNode.is_map())
            {
                if (!colorNode.find_child("enabled").invalid())
                {
                    std::string boolRes {colorNode["enabled"].val().str, colorNode["enabled"].val().len};
                    m_colorEnabled = (boolRes == "true");
                }
                if (!colorNode.find_child("set_key_color").invalid())
                {
                    m_keyTextColor = validTextColorCode(colorNode["set_key_color"].val());
                }
                if (!colorNode.find_child("set_val_color").invalid())
                {
                    m_valTextColor = validTextColorCode(colorNode["set_val_color"].val());
                }
            }
        }
        if (!configNode.find_child("image").invalid())
        {
            if (const c4::yml::ConstNodeRef imageNode {configNode["image"]}; imageNode.is_map())
            {
                if (!imageNode.find_child("enabled").invalid())
                {
                    std::string boolRes {imageNode["enabled"].val().str, imageNode["enabled"].val().len};
                    m_imageEnabled = (boolRes == "true");
                }
            }
        }
        if (!configNode.find_child("formatting").invalid())
        {
            if (const c4::yml::ConstNodeRef formattingNode {configNode["formatting"]}; formattingNode.is_map())
            {
                if (!formattingNode.find_child("indent_length").invalid())
                {
                    std::string numberRes {
                        formattingNode["indent_length"].val().str,
                        formattingNode["indent_length"].val().len
                    };
                    m_indentLength = std::stoi(numberRes);
                }
            }
        }
    }
}

std::string Configuration::findConfigFile()
{
#ifdef IS_DEBUG
    const std::string configDir {"../presets/config.yaml"};
    if (std::filesystem::exists(configDir)) return configDir;
#else
    std::string configDir {std::getenv("HOME")};
    if (configDir.empty()) throw std::runtime_error("Error: \"HOME\" environment variable can't be found");
    configDir += "/.config/mtgfetch/config.yaml";
    if (std::filesystem::exists(configDir)) return configDir;
#endif
    throw std::runtime_error("Error: Config File not found\n" + configDir + " does not exist.");
}

std::string Configuration::validTextColorCode(const c4::csubstr color)
{
    const std::string keyTextColorString {std::string(color.str, color.len)};
    bool validColorCode {false};
    if (keyTextColorString.size() <= 3 && !keyTextColorString.empty())
    {
        validColorCode = true;
        for (const char& number : keyTextColorString)
        {
            if (!std::isdigit(number))
            {
                validColorCode = false;
                break;
            }
        }
    }
    return validColorCode ? "\033[1;38;5;" + keyTextColorString + 'm' : "";
}
