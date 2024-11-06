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

    // check if options values are set
    if (const c4::yml::ConstNodeRef colorEnabledNode = m_configTree["options"]["color"]["enabled"];
        colorEnabledNode.id() != c4::yml::NONE && colorEnabledNode.is_keyval())
    {
        std::string boolRes {colorEnabledNode.val().str, colorEnabledNode.val().len};
        m_colorEnabled = (boolRes == "true");
    }
    if (const c4::yml::ConstNodeRef setKeyColorNode = m_configTree["options"]["color"]["set_key_color"];
        setKeyColorNode.id() != c4::yml::NONE && setKeyColorNode.is_keyval())
    {
        m_keyTextColor = validTextColorCode(setKeyColorNode.val());
    }
    if (const c4::yml::ConstNodeRef setValColorNode = m_configTree["options"]["color"]["set_val_color"];
        setValColorNode.id() != c4::yml::NONE && setValColorNode.is_keyval())
    {
        m_valTextColor = validTextColorCode(setValColorNode.val());
    }
    if (const c4::yml::ConstNodeRef imageEnabledNode = m_configTree["options"]["image"]["enabled"];
        imageEnabledNode.id() != c4::yml::NONE && imageEnabledNode.is_keyval())
    {
        std::string boolRes {imageEnabledNode.val().str, imageEnabledNode.val().len};
        m_imageEnabled = (boolRes == "true");
    }
    if (const c4::yml::ConstNodeRef indentLengthNode = m_configTree["options"]["formatting"]["indent_length"];
        indentLengthNode.id() != c4::yml::NONE && indentLengthNode.is_keyval())
    {
        std::string numberRes {indentLengthNode.val().str, indentLengthNode.val().len};
        m_indentLength = std::stoi(numberRes);
    }
}

std::string Configuration::findConfigFile()
{
    if (std::filesystem::exists("../presets/config.yaml")) return "../presets/config.yaml";
    throw std::runtime_error("Error: Config File not found");
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
