#pragma once

#include <QLayout>
#include <QWidget>
#include <string>
#include <vector>

/**
 * @brief Miscellaneous utility functions.
 */
namespace Util {
    /**
     * @brief Get a random number in the range [min, max].
     *
     * @param min The start of the number range.
     * @param max The end of the number range.
     * @return int - The random number.
     */
    int random(int min, int max);
    /**
     * @brief Get a random, commonly used user agent.
     *
     * @param limit Limit the search to the top N most commonly used user agents.
     * @return std::string - The user agent.
     */
    std::string getRandomUA(int limit = 3);
    /**
     * @brief Split a string into a vector of strings based on a delimiter.
     * Does not remove trailing empty strings if the source string ends with the delimiter.
     *
     * @param string The string getting split
     * @param delimiter The delimiter
     * @return std::vector<std::string> - The resulting vector of strings.
     */
    std::vector<std::string> split(const std::string& string, const std::string& delimiter);
    /**
     * @brief Check if a string contains a valid ntfy domain
     *
     * @param domain The domain to check
     */
    bool isDomain(const std::string& domain);
    /**
     * @brief Check if a string contains a valid ntfy topic
     *
     * @param topic The topic to check
     */
    bool isTopic(const std::string& topic);
    /**
     * @brief Change the visibility of an entire Qt layout.
     * @warning Only works with widgets and layouts. Spacers and other elements are unsupported.
     */
    void setLayoutVisibility(QLayout* layout, bool visible);
}
