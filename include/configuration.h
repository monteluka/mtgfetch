#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include "../lib/rapidyaml-0.7.2.hpp"


class Configuration
{
public:
    Configuration();

    ~Configuration() = default;

    [[nodiscard]] bool getColorEnabledOption() const { return m_colorEnabled; }
    [[nodiscard]] bool getImageEnabledOption() const { return m_imageEnabled; }
    [[nodiscard]] std::string getTextColor() const { return m_textColor; }
    [[nodiscard]] c4::yml::Tree getConfigTree() const { return m_configTree; }

private:
    bool m_colorEnabled {false};
    bool m_imageEnabled {false};
    std::string m_textColor;
    c4::yml::Tree m_configTree {};

    std::string findConfigFile();
};


#endif //CONFIGURATION_H
