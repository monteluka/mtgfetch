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
    if (const c4::yml::ConstNodeRef setColorNode = m_configTree["options"]["color"]["set_color"];
        setColorNode.id() != c4::yml::NONE)
    {
        m_textColor = std::string {setColorNode.val().str, setColorNode.val().len};
    }
    if (const c4::yml::ConstNodeRef imageEnabledNode = m_configTree["options"]["image"]["enabled"];
        imageEnabledNode.id() != c4::yml::NONE)
    {
        std::string boolRes {imageEnabledNode.val().str, imageEnabledNode.val().len};
        m_colorEnabled = (boolRes == "true");
    }
}

std::string Configuration::findConfigFile()
{
    if (std::filesystem::exists("../presets/config.yaml")) return "../presets/config.yaml";
    std::cerr << "Could not find config file" << std::endl;
}
