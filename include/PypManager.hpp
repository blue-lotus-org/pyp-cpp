#ifndef PYP_MANAGER_HPP
#define PYP_MANAGER_HPP

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <nlohmann/json.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using json = nlohmann::json;

/**
 * @brief Structure representing a Python virtual environment
 */
struct Environment {
    std::string name;
    fs::path path;
    bool isActive;
    bool isDefault;
    std::string pythonVersion;
    std::string createdAt;
    std::string description;
};

/**
 * @brief Main class for managing secure Python virtual environments
 * 
 * This class provides functionality to create, manage, and delete
 * Python virtual environments with enhanced security features.
 */
class PypManager {
public:
    /**
     * @brief Construct a new PypManager instance
     * @param customEnvsDir Optional custom directory for environments
     * @param customConfigPath Optional custom path for config file
     */
    explicit PypManager(
        const std::optional<fs::path>& customEnvsDir = std::nullopt,
        const std::optional<fs::path>& customConfigPath = std::nullopt
    );

    /**
     * @brief Destructor
     */
    ~PypManager();

    // Disable copying
    PypManager(const PypManager&) = delete;
    PypManager& operator=(const PypManager&) = delete;

    // Allow moving
    PypManager(PypManager&&) noexcept;
    PypManager& operator=(PypManager&&) noexcept;

    // ==================== Core Operations ====================

    /**
     * @brief Create a new virtual environment
     * @param envName Name of the environment (must be valid)
     * @param pythonVersion Python version to use (e.g., "3.11", "3.10")
     * @param upgrade Upgrade existing environment if it exists
     * @return true if environment was created successfully
     */
    bool buildEnvironment(
        const std::string& envName,
        const std::string& pythonVersion = "python3",
        bool upgrade = false
    );

    /**
     * @brief Activate a virtual environment
     * @param envName Name of the environment to activate
     * @return true if activation was successful
     */
    bool activateEnvironment(const std::string& envName);

    /**
     * @brief Deactivate the current environment
     * @return true if deactivation was successful
     */
    bool deactivateEnvironment();

    /**
     * @brief List all available environments
     * @return Vector of Environment structures
     */
    std::vector<Environment> listEnvironments() const;

    /**
     * @brief Remove an environment
     * @param envName Name of the environment to remove
     * @param force Force removal without confirmation
     * @return true if removal was successful
     */
    bool removeEnvironment(const std::string& envName, bool force = false);

    /**
     * @brief Get information about a specific environment
     * @param envName Name of the environment
     * @return Optional containing Environment info if found
     */
    std::optional<Environment> getEnvironmentInfo(const std::string& envName) const;

    /**
     * @brief Check if an environment exists
     * @param envName Name of the environment
     * @return true if environment exists
     */
    bool environmentExists(const std::string& envName) const;

    /**
     * @brief Get the currently active environment
     * @return Optional containing the active environment name
     */
    std::optional<std::string> getActiveEnvironment() const;

    // ==================== Utility Methods ====================

    /**
     * @brief Validate an environment name
     * @param envName Name to validate
     * @return true if name is valid
     */
    static bool isValidEnvName(const std::string& envName);

    /**
     * @brief Get the path to Python executable in an environment
     * @param envName Name of the environment
     * @return Path to Python executable
     */
    fs::path getPythonPath(const std::string& envName) const;

    /**
     * @brief Get the path to pip executable in an environment
     * @param envName Name of the environment
     * @return Path to pip executable
     */
    fs::path getPipPath(const std::string& envName) const;

    /**
     * @brief Run a command in a specific environment
     * @param envName Name of the environment
     * @param command Command to run
     * @return Exit code of the command
     */
    int runInEnvironment(const std::string& envName, const std::string& command);

    /**
     * @brief Export environment details to a file
     * @param envName Name of the environment
     * @param outputPath Path to save the export
     * @return true if export was successful
     */
    bool exportEnvironment(const std::string& envName, const fs::path& outputPath);

    /**
     * @brief Import an environment from export
     * @param importPath Path to the import file
     * @param newName New name for the imported environment
     * @return true if import was successful
     */
    bool importEnvironment(const fs::path& importPath, const std::string& newName);

    // ==================== Configuration Methods ====================

    /**
     * @brief Set a default environment
     * @param envName Name of the environment to set as default
     * @return true if operation was successful
     */
    bool setDefaultEnvironment(const std::string& envName);

    /**
     * @brief Get the default environment name
     * @return Name of the default environment or empty string
     */
    std::string getDefaultEnvironment() const;

    /**
     * @brief Add a description to an environment
     * @param envName Name of the environment
     * @param description Description to set
     * @return true if operation was successful
     */
    bool setEnvironmentDescription(const std::string& envName, const std::string& description);

    /**
     * @brief Get the configuration directory path
     * @return Path to the configuration directory
     */
    fs::path getConfigDir() const;

    /**
     * @brief Get the environments directory path
     * @return Path to the environments directory
     */
    fs::path getEnvsDir() const;

    // ==================== Initialization ====================

    /**
     * @brief Initialize the pyp configuration
     * @return true if initialization was successful
     */
    bool initialize();

    /**
     * @brief Clean up orphaned environments
     * @return Number of environments cleaned up
     */
    int cleanupOrphanedEnvironments();

private:
    fs::path configPath_;
    fs::path envsDir_;
    json config_;
    std::string activeEnv_;

    // Private helper methods
    void loadConfig();
    void saveConfig();
    void ensureSecurePermissions(const fs::path& path, mode_t mode);
    fs::path resolveEnvironmentPath(const std::string& envName) const;
    bool createSecureDirectory(const fs::path& path);
    bool removeSecureDirectory(const fs::path& path);
    json environmentToJson(const Environment& env) const;
    Environment jsonToEnvironment(const std::string& name, const json& j) const;
    std::string getCurrentTimestamp() const;
    bool executeCommand(const std::string& command, int& exitCode);
    std::string getShellActivationCommand(const fs::path& envPath) const;
};

#endif // PYP_MANAGER_HPP
