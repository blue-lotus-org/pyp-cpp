#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "PypManager.hpp"

/**
 * @brief Structure representing a parsed command
 */
struct Command {
    std::string name;
    std::vector<std::string> args;
    std::map<std::string, std::string> options;
};

/**
 * @brief Result of command execution
 */
struct CommandResult {
    bool success;
    int exitCode;
    std::string message;
    
    CommandResult(bool success = true, int exitCode = 0, const std::string& message = "")
        : success(success), exitCode(exitCode), message(message) {}
};

/**
 * @brief Class handling command-line interface and command execution
 */
class CommandHandler {
public:
    /**
     * @brief Construct a new CommandHandler
     * @param manager Reference to the PypManager instance
     */
    explicit CommandHandler(PypManager& manager);

    /**
     * @brief Destructor
     */
    ~CommandHandler();

    // Disable copying
    CommandHandler(const CommandHandler&) = delete;
    CommandHandler& operator=(const CommandHandler&) = delete;

    /**
     * @brief Process command-line arguments
     * @param argc Argument count
     * @param argv Argument vector
     * @return CommandResult indicating success or failure
     */
    CommandResult processArgs(int argc, char* argv[]);

    /**
     * @brief Display help information
     */
    void showHelp() const;

    /**
     * @brief Display version information
     */
    void showVersion() const;

    /**
     * @brief Display usage information
     */
    void showUsage() const;

private:
    PypManager& manager_;

    // Command definitions
    static const std::map<std::string, std::vector<std::string>> COMMAND_ALIASES;
    static const std::map<std::string, std::string> COMMAND_DESCRIPTIONS;
    static const std::map<std::string, std::vector<std::string>> COMMAND_USAGE;

    /**
     * @brief Parse command-line arguments into a Command structure
     * @param argc Argument count
     * @param argv Argument vector
     * @return Parsed Command
     */
    Command parseArgs(int argc, char* argv[]);

    /**
     * @brief Resolve command aliases
     * @param command Original command name
     * @return Resolved command name
     */
    std::string resolveCommand(const std::string& command) const;

    /**
     * @brief Execute a command
     * @param command Command to execute
     * @return CommandResult
     */
    CommandResult executeCommand(const Command& command);

    // Command execution methods
    CommandResult cmdBuild(const Command& cmd);
    CommandResult cmdActivate(const Command& cmd);
    CommandResult cmdDeactivate(const Command& cmd);
    CommandResult cmdList(const Command& cmd);
    CommandResult cmdRemove(const Command& cmd);
    CommandResult cmdInfo(const Command& cmd);
    CommandResult cmdRun(const Command& cmd);
    CommandResult cmdExport(const Command& cmd);
    CommandResult cmdImport(const Command& cmd);
    CommandResult cmdSetDefault(const Command& cmd);
    CommandResult cmdCleanup(const Command& cmd);
    CommandResult cmdInit(const Command& cmd);

    /**
     * @brief Print a formatted table of environments
     * @param environments List of environments to display
     */
    void printEnvironmentTable(const std::vector<Environment>& environments) const;

    /**
     * @brief Format an environment for display
     * @param env Environment to format
     * @return Formatted string
     */
    std::string formatEnvironment(const Environment& env) const;

    /**
     * @brief Get help for a specific command
     * @param command Command name
     */
    void showCommandHelp(const std::string& command) const;

    /**
     * @brief Validate command arguments
     * @param command Command being executed
     * @param cmd Parsed command
     * @param minArgs Minimum number of arguments required
     * @param maxArgs Maximum number of arguments allowed (-1 for unlimited)
     * @return true if valid
     */
    bool validateArgs(const std::string& command, const Command& cmd, int minArgs, int maxArgs) const;
};

#endif // COMMAND_HANDLER_HPP
