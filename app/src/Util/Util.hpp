#pragma once

#include <QLayout>
#include <QWidget>
#include <ctime>
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

    namespace Strings {
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
         * @brief Transform a string to upper case.
         *
         * @param str The string
         */
        void toUpper(std::string& str);

        /**
         * @brief Transform a string to upper case.
         *
         * @param str The string
         * @return std::string - The string but upper case.
         */
        std::string getUpper(const std::string& str);

        /**
         * @brief Transform a string to lower case.
         *
         * @param str The string
         */
        void toLower(std::string& str);

        /**
         * @brief Transform a string to lower case.
         *
         * @param str The string
         * @return std::string - The string but lower case.
         */
        std::string getLower(const std::string& str);

        /**
         * @brief Check if a string contains a substring.
         *
         * @param str The main string
         * @param substr The substring that the main string is being checked for
         */
        bool contains(const std::string& str, const std::string& substr);

        /**
         * @brief Check if a string starts with a substring.
         *
         * @param str The main string
         * @param substr The substring that the main string is being checked for
         */
        bool startsWith(const std::string& str, const std::string& substr);

        /**
         * @brief Check if a string ends with a substring.
         *
         * @param str The main string
         * @param substr The substring that the main string is being checked for
         */
        bool endsWith(const std::string& str, const std::string& substr);
    }

    /**
     * @brief Get a random, commonly used user agent.
     *
     * @param limit Limit the search to the top N most commonly used user agents.
     * @return std::string - The user agent.
     */
    std::string getRandomUA(int limit = 3);

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

    /**
     * @brief Compares three-part version strings in the format `major.minor.patch`.
     *
     * @param first The first version string used in the comparison
     * @param second The second version string used in the comparison
     * @return int - `-1` if `first` is bigger, `0` if they are equal and `1` if `second` is bigger.
     * @throws std::invalid_argument - Thrown when either of the strings can't be recognised as a valid version.
     */
    int versionCompare(const std::string& first, const std::string& second);

    /**
     * @brief Color related functions for dynamic theme handling
     */
    namespace Colors {
        const QColor textColor();
        const QColor textColorSuccess();
        const QColor textColorFailure();
        const QColor buttonColor();
        const QColor buttonColorSuccess();
        const QColor buttonColorFailure();
        const QColor buttonTextColor();
        const QColor buttonTextColorSuccess();
        const QColor buttonTextColorFailure();
    }

    /**
     * @brief Generate hash representing a topic of a specific ntfy server.
     *
     * @param domain The domain of the ntfy server
     * @param topic The topic on the ntfy server
     * @return std::string - hash
     */
    std::string topicHash(const std::string& domain, const std::string& topic);

    /**
     * @brief Convert unix time to a formatted string for display.
     *
     * @param time Some unix time
     * @return std::string - A formatted string of the given unix time
     */
    std::string timeToString(const std::time_t& time);
}
