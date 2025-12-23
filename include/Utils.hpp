#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Utility class providing common helper functions
 */
class Utils {
public:
    // ==================== Output Functions ====================

    /**
     * @brief Print a success message in green
     * @param message Message to print
     */
    static void printSuccess(const std::string& message);

    /**
     * @brief Print an error message in red
     * @param message Message to print
     */
    static void printError(const std::string& message);

    /**
     * @brief Print a warning message in yellow
     * @param message Message to print
     */
    static void printWarning(const std::string& message);

    /**
     * @brief Print an info message in blue
     * @param message Message to print
     */
    static void printInfo(const std::string& message);

    /**
     * @brief Print a debug message (only in debug builds)
     * @param message Message to print
     */
    static void printDebug(const std::string& message);

    /**
     * @brief Print a section header
     * @param title Title to print
     */
    static void printHeader(const std::string& title);

    // ==================== String Functions ====================

    /**
     * @brief Escape a string for safe use in shell commands
     * @param input String to escape
     * @return Escaped string safe for shell use
     */
    static std::string escapeShellArg(const std::string& input);

    /**
     * @brief Trim whitespace from both ends of a string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trim(const std::string& str);

    /**
     * @brief Convert a string to lowercase
     * @param str String to convert
     * @return Lowercase string
     */
    static std::string toLower(const std::string& str);

    /**
     * @brief Convert a string to uppercase
     * @param str String to convert
     * @return Uppercase string
     */
    static std::string toUpper(const std::string& str);

    /**
     * @brief Check if a string starts with a prefix
     * @param str String to check
     * @param prefix Prefix to look for
     * @return true if string starts with prefix
     */
    static bool startsWith(const std::string& str, const std::string& prefix);

    /**
     * @brief Check if a string ends with a suffix
     * @param str String to check
     * @param suffix Suffix to look for
     * @return true if string ends with suffix
     */
    static bool endsWith(const std::string& str, const std::string& suffix);

    /**
     * @brief Split a string by a delimiter
     * @param str String to split
     * @param delimiter Delimiter character
     * @return Vector of split parts
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);

    /**
     * @brief Join a vector of strings with a separator
     * @param parts Vector of strings to join
     * @param separator Separator to use
     * @return Joined string
     */
    static std::string join(const std::vector<std::string>& parts, const std::string& separator);

    // ==================== File Functions ====================

    /**
     * @brief Check if a file exists
     * @param path Path to check
     * @return true if file exists
     */
    static bool fileExists(const fs::path& path);

    /**
     * @brief Check if a path is a directory
     * @param path Path to check
     * @return true if path is a directory
     */
    static bool isDirectory(const fs::path& path);

    /**
     * @brief Read the entire contents of a file
     * @param path Path to file
     * @return File contents as string
     */
    static std::string readFile(const fs::path& path);

    /**
     * @brief Write content to a file
     * @param path Path to file
     * @param content Content to write
     * @return true if write was successful
     */
    static bool writeFile(const fs::path& path, const std::string& content);

    /**
     * @brief Get the size of a file
     * @param path Path to file
     * @return File size in bytes, or -1 on error
     */
    static long long getFileSize(const fs::path& path);

    /**
     * @brief Create a temporary file
     * @param prefix Prefix for the temporary file name
     * @return Path to the created file
     */
    static fs::path createTempFile(const std::string& prefix);

    // ==================== System Functions ====================

    /**
     * @brief Get the current user's home directory
     * @return Home directory path
     */
    static fs::path getHomeDirectory();

    /**
     * @brief Get the current working directory
     * @return Current working directory path
     */
    static fs::path getCurrentDirectory();

    /**
     * @brief Get the current user's username
     * @return Username string
     */
    static std::string getUsername();

    /**
     * @brief Execute a system command and capture output
     * @param command Command to execute
     * @param output Captured output from command
     * @return Exit code of the command
     */
    static int executeCommand(const std::string& command, std::string& output);

    /**
     * @brief Execute a command and get just the exit code
     * @param command Command to execute
     * @return Exit code
     */
    static int executeCommand(const std::string& command);

    /**
     * @brief Check if a command exists in PATH
     * @param command Command name to check
     * @return true if command exists
     */
    static bool commandExists(const std::string& command);

    /**
     * @brief Get the current timestamp as a string
     * @return Timestamp string
     */
    static std::string getTimestamp();

    /**
     * @brief Sleep for a specified duration
     * @param milliseconds Duration in milliseconds
     */
    static void sleep(int milliseconds);

    // ==================== Validation Functions ====================

    /**
     * @brief Check if a string is a valid environment name
     * @param name Name to validate
     * @return true if valid
     */
    static bool isValidEnvName(const std::string& name);

    /**
     * @brief Check if a string is a valid path
     * @param path Path to validate
     * @return true if valid
     */
    static bool isValidPath(const std::string& path);

    // ==================== Format Functions ====================

    /**
     * @brief Format a file size for human reading
     * @param bytes Size in bytes
     * @return Formatted string
     */
    static std::string formatSize(long long bytes);

    /**
     * @brief Format a duration for human reading
     * @param milliseconds Duration in milliseconds
     * @return Formatted string
     */
    static std::string formatDuration(long long milliseconds);

    // ==================== Color Codes ====================

    static const std::string RESET;
    static const std::string RED;
    static const std::string GREEN;
    static const std::string YELLOW;
    static const std::string BLUE;
    static const std::string MAGENTA;
    static const std::string CYAN;
    static const std::string WHITE;
    static const std::string BOLD;
};

#endif // UTILS_HPP
