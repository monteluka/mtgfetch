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
    [[nodiscard]] int getIndentLength() const { return m_indentLength; }
    [[nodiscard]] int getTerminalWidth() const { return m_terminalWidth; }
    [[nodiscard]] std::string getKeyTextColor() const { return m_keyTextColor; }
    [[nodiscard]] std::string getValTextColor() const { return m_valTextColor; }
    [[nodiscard]] c4::yml::Tree getConfigTree() const { return m_configTree; }

private:
    bool m_colorEnabled {false};
    bool m_imageEnabled {false};
    int m_indentLength {2};
    int m_terminalWidth {};
    std::string m_keyTextColor;
    std::string m_valTextColor;
    c4::yml::Tree m_configTree {};

    static std::string findConfigFile();
    static std::string validTextColorCode(c4::csubstr color);
};


#endif //CONFIGURATION_H
