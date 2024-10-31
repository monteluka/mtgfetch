#include "../include/configuration.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>

Configuration::Configuration()
{
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
    configFile.close();

    m_configTree = {c4::yml::parse_in_arena(c4::to_csubstr(configContents))};

    // check if options values are set
    if (const c4::yml::ConstNodeRef colorEnabledNode = m_configTree["options"]["color"]["enabled"];
        colorEnabledNode.id() != c4::yml::NONE)
    {
        std::string boolRes {colorEnabledNode.val().str, colorEnabledNode.val().len};
        m_colorEnabled = (boolRes == "true");
    }
    if (const c4::yml::ConstNodeRef setKeyColorNode = m_configTree["options"]["color"]["set_key_color"];
        setKeyColorNode.id() != c4::yml::NONE)
    {
        m_keyTextColor = std::string {setKeyColorNode.val().str, setKeyColorNode.val().len};
    }
    if (const c4::yml::ConstNodeRef setValColorNode = m_configTree["options"]["color"]["set_val_color"];
        setValColorNode.id() != c4::yml::NONE)
    {
        m_valTextColor = std::string {setValColorNode.val().str, setValColorNode.val().len};
    }
    if (const c4::yml::ConstNodeRef imageEnabledNode = m_configTree["options"]["image"]["enabled"];
        imageEnabledNode.id() != c4::yml::NONE)
    {
        std::string boolRes {imageEnabledNode.val().str, imageEnabledNode.val().len};
        m_imageEnabled = (boolRes == "true");
    }
    if (const c4::yml::ConstNodeRef indentLengthNode = m_configTree["options"]["formatting"]["indent_length"];
        indentLengthNode.id() != c4::yml::NONE)
    {
        std::string numberRes {indentLengthNode.val().str, indentLengthNode.val().len};
        m_indentLength = std::stoi(numberRes);
    }
}

std::string Configuration::findConfigFile()
{
    if (std::filesystem::exists("../presets/config.yaml")) return "../presets/config.yaml";
    std::cerr << "Could not find config file" << std::endl;
}
