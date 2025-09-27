#pragma once

#include <QAbstractButton>
#include <QLabel>
#include <QLayout>
#include <QWidget>
#include <ctime>
#include <string>
#include <vector>

/**
 * @brief Miscellaneous utility functions
 */
namespace Util {
    /**
     * @brief String related utilities
     */
    namespace Strings {
        /**
         * @brief Split a string into a vector of strings based on a delimiter.
         * Does not remove trailing empty strings if the source string ends with the delimiter.
         *
         * @param string The string getting split
         * @param delimiter The delimiter
         * @return std::vector<std::string> - The resulting vector of strings.
         */
        std::vector<std::string> split(std::string_view string, std::string_view delimiter);

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
        std::string getUpper(std::string_view str);

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
        std::string getLower(std::string_view str);

        /**
         * @brief Check if a string contains a substring.
         *
         * @param str The main string
         * @param substr The substring that the main string is being checked for
         */
        bool contains(std::string_view str, std::string_view substr);

        /**
         * @brief Check if a string starts with a substring.
         *
         * @param str The main string
         * @param substr The substring that the main string is being checked for
         */
        bool startsWith(std::string_view str, std::string_view substr);

        /**
         * @brief Check if a string ends with a substring.
         *
         * @param str The main string
         * @param substr The substring that the main string is being checked for
         */
        bool endsWith(std::string_view str, std::string_view substr);
    }

    /**
     * @brief Color related functions for dynamic theme handling
     */
    namespace Colors {
        enum ColorMode { Failure, Success, Normal };
        void setButtonColor(QAbstractButton& button, ColorMode mode = ColorMode::Normal);
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
     * @brief Utilities related to the environment of the app
     */
    namespace Env {
        /**
         * @brief Fetch the contents of an environment variable
         *
         * @param var The environment variable
         * @return std::optional<std::string> - The optional contents of the environment variable
         */
        std::optional<std::string> getVar(const std::string& var);

        /**
         * @brief Common environment variables used in Ntfy Desktop
         */
        namespace Commons {
            /**
             * @brief Is debug mode on?
             */
            const std::optional<std::string> debug = getVar("ND_DEBUG");
        }
    }

    /**
     * @brief Get a random number in the range [min, max]
     *
     * @param min The start of the number range
     * @param max The end of the number range
     * @return int - The random number
     */
    int random(int min, int max);

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
     * @brief Change the visibility of an entire Qt layout
     * @warning Only works with widgets and layouts; Spacers and other elements are unsupported
     */
    void setLayoutVisibility(QLayout* layout, bool visible);

    /**
     * @brief Compares three-part version strings in the format `major.minor.patch`
     *
     * @param first The first version string used in the comparison
     * @param second The second version string used in the comparison
     * @return int - `-1` if `first` is bigger, `0` if they are equal and `1` if `second` is bigger
     * @throws std::invalid_argument - Thrown when either of the strings can't be recognised as a valid version
     */
    int versionCompare(std::string_view first, std::string_view second);

    /**
     * @brief Generate hash representing a topic of a specific ntfy server
     *
     * @param domain The domain of the ntfy server
     * @param topic The topic on the ntfy server
     * @return std::string - hash
     */
    std::string topicHash(std::string_view domain, std::string_view topic);

    /**
     * @brief Convert unix time to a formatted string for display
     *
     * @param time Some unix time
     * @return std::string - A formatted string of the given unix time
     */
    std::string timeToString(const std::time_t& time);
}
