#pragma once

#include <string>

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
}
