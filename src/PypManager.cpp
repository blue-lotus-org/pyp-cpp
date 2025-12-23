#include "PypManager.hpp"
#include "Utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <libgen.h>

PypManager::PypManager(
    const std::optional<fs::path>& customEnvsDir,
    const std::optional<fs::path>& customConfigPath
) : activeEnv_("") {
    // Determine home directory
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        struct passwd* pw = getpwuid(getuid());
        homeDir = pw ? pw->pw_dir : "/tmp";
    }
    fs::path homePath(homeDir);

    // Set paths
    if (customConfigPath.has_value()) {
        configPath_ = customConfigPath.value();
    } else {
        configPath_ = homePath / ".pyp_config.json";
    }

    if (customEnvsDir.has_value()) {
        envsDir_ = customEnvsDir.value();
    } else {
        envsDir_ = homePath / ".pyp_envs";
    }

    // Load existing configuration
    loadConfig();

    // Check for active environment
    const char* activeEnv = getenv("PYP_CURRENT_ENV");
    if (activeEnv) {
        activeEnv_ = activeEnv;
    }
}

PypManager::~PypManager() {
    // Destructor
}

PypManager::PypManager(PypManager&& other) noexcept
    : configPath_(std::move(other.configPath_)),
      envsDir_(std::move(other.envsDir_)),
      config_(std::move(other.config_)),
      activeEnv_(std::move(other.activeEnv_)) {
    other.activeEnv_ = "";
}

PypManager& PypManager::operator=(PypManager&& other) noexcept {
    if (this != &other) {
        configPath_ = std::move(other.configPath_);
        envsDir_ = std::move(other.envsDir_);
        config_ = std::move(other.config_);
        activeEnv_ = std::move(other.activeEnv_);
        other.activeEnv_ = "";
    }
    return *this;
}

bool PypManager::initialize() {
    try {
        // Create environments directory with secure permissions
        if (!createSecureDirectory(envsDir_)) {
            Utils::printError("Failed to create environments directory: " + envsDir_.string());
            return false;
        }

        // Ensure config file exists with secure permissions
        if (!fs::exists(configPath_)) {
            std::ofstream configFile(configPath_);
            if (!configFile.is_open()) {
                Utils::printError("Failed to create config file: " + configPath_.string());
                return false;
            }
            configFile << "{}";
            configFile.close();
        }

        // Set secure permissions on config file
        ensureSecurePermissions(configPath_, S_IRUSR | S_IWUSR); // 0o600

        // Initialize config structure if needed
        if (!config_.contains("environments")) {
            config_["environments"] = json::object();
        }
        if (!config_.contains("settings")) {
            config_["settings"] = json::object();
        }
        if (!config_["settings"].contains("default")) {
            config_["settings"]["default"] = "";
        }

        saveConfig();

        Utils::printSuccess("pyp initialized successfully");
        Utils::printInfo("Environments directory: " + envsDir_.string());
        Utils::printInfo("Config file: " + configPath_.string());

        return true;
    } catch (const std::exception& e) {
        Utils::printError("Initialization failed: " + std::string(e.what()));
        return false;
    }
}

void PypManager::loadConfig() {
    try {
        if (fs::exists(configPath_)) {
            std::ifstream configFile(configPath_);
            if (configFile.is_open()) {
                config_ = json::parse(configFile);
                configFile.close();
            } else {
                config_ = json::object();
            }
        } else {
            config_ = json::object();
        }

        // Ensure required structure
        if (!config_.contains("environments")) {
            config_["environments"] = json::object();
        }
        if (!config_.contains("settings")) {
            config_["settings"] = json::object();
        }
    } catch (const std::exception& e) {
        Utils::printWarning("Failed to load config: " + std::string(e.what()));
        config_ = json::object();
        config_["environments"] = json::object();
        config_["settings"] = json::object();
    }
}

void PypManager::saveConfig() {
    try {
        // Create a temporary file for atomic write
        fs::path tempPath = configPath_.string() + ".tmp";
        std::ofstream tempFile(tempPath);
        if (tempFile.is_open()) {
            tempFile << config_.dump(2);
            tempFile.close();

            // Rename to actual config file (atomic operation)
            fs::rename(tempPath, configPath_);

            // Set secure permissions
            ensureSecurePermissions(configPath_, S_IRUSR | S_IWUSR);
        } else {
            Utils::printError("Failed to save config file");
        }
    } catch (const std::exception& e) {
        Utils::printError("Failed to save config: " + std::string(e.what()));
    }
}

bool PypManager::createSecureDirectory(const fs::path& path) {
    try {
        // Create directory with default permissions first
        fs::create_directories(path);

        // Then set restrictive permissions
        ensureSecurePermissions(path, S_IRWXU); // 0o700

        return true;
    } catch (const std::exception& e) {
        Utils::printError("Failed to create directory: " + std::string(e.what()));
        return false;
    }
}

bool PypManager::removeSecureDirectory(const fs::path& path) {
    try {
        // First, remove all contents recursively
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_directory()) {
                fs::remove_all(entry.path());
            } else {
                fs::remove(entry.path());
            }
        }

        // Then remove the directory itself
        fs::remove(path);

        return true;
    } catch (const std::exception& e) {
        Utils::printError("Failed to remove directory: " + std::string(e.what()));
        return false;
    }
}

void PypManager::ensureSecurePermissions(const fs::path& path, mode_t mode) {
    try {
        // Use chmod to set permissions
        std::string pathStr = path.string();
        chmod(pathStr.c_str(), mode);
    } catch (const std::exception& e) {
        Utils::printWarning("Failed to set permissions on " + path.string() + ": " + e.what());
    }
}

fs::path PypManager::resolveEnvironmentPath(const std::string& envName) const {
    fs::path resolvedPath = envsDir_ / envName;
    return fs::weakly_canonical(resolvedPath);
}

bool PypManager::isValidEnvName(const std::string& envName) {
    if (envName.empty() || envName.length() > 64) {
        return false;
    }

    // Check for valid characters: alphanumeric, underscore, hyphen
    for (char c : envName) {
        if (!std::isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }

    // Check for dangerous patterns
    if (envName == "." || envName == ".." || envName[0] == '.') {
        return false;
    }

    return true;
}

bool PypManager::buildEnvironment(
    const std::string& envName,
    const std::string& pythonVersion,
    bool upgrade
) {
    // Validate environment name
    if (!isValidEnvName(envName)) {
        Utils::printError("Invalid environment name: " + envName);
        Utils::printInfo("Environment names must be alphanumeric (a-z, A-Z, 0-9) with underscores and hyphens allowed.");
        return false;
    }

    fs::path envPath = resolveEnvironmentPath(envName);

    // Check if environment already exists
    if (fs::exists(envPath)) {
        if (!upgrade) {
            Utils::printError("Environment '" + envName + "' already exists");
            Utils::printInfo("Use --upgrade to rebuild the environment");
            return false;
        }
        Utils::printInfo("Upgrading existing environment: " + envName);
    } else {
        // Create environment directory with secure permissions
        if (!createSecureDirectory(envPath)) {
            return false;
        }
    }

    // Build the virtual environment
    std::string command;
    if (pythonVersion == "python3" || pythonVersion.empty()) {
        command = "python3 -m venv";
    } else if (pythonVersion == "python" || pythonVersion == "python2") {
        command = pythonVersion + " -m venv";
    } else {
        command = pythonVersion + " -m venv";
    }

    if (upgrade) {
        command += " --clear";
    }

    command += " " + Utils::escapeShellArg(envPath.string());

    Utils::printInfo("Creating virtual environment with " + pythonVersion + "...");

    int exitCode = 0;
    if (!executeCommand(command, exitCode)) {
        Utils::printError("Failed to create virtual environment");
        return false;
    }

    // Set secure permissions on the environment directory
    ensureSecurePermissions(envPath, S_IRWXU); // 0o700

    // Set secure permissions on the pyvenv.cfg file
    fs::path pyvenvCfg = envPath / "pyvenv.cfg";
    if (fs::exists(pyvenvCfg)) {
        ensureSecurePermissions(pyvenvCfg, S_IRUSR | S_IWUSR); // 0o600
    }

    // Get Python version
    std::string pythonFullVersion;
    std::string versionCommand = Utils::escapeShellArg((envPath / "bin" / "python").string()) + " --version";
    int versionExitCode = 0;
    if (executeCommand(versionCommand, versionExitCode)) {
        // Capture the version
        command = Utils::escapeShellArg((envPath / "bin" / "python").string()) + " --version 2>&1";
        FILE* pipe = popen(command.c_str(), "r");
        if (pipe) {
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                pythonFullVersion = std::string(buffer);
                // Remove trailing newline
                if (!pythonFullVersion.empty() && pythonFullVersion.back() == '\n') {
                    pythonFullVersion.pop_back();
                }
            }
            pclose(pipe);
        }
    }

    // Update configuration
    json envConfig = json::object();
    envConfig["path"] = envPath.string();
    envConfig["python"] = pythonVersion;
    envConfig["python_full_version"] = pythonFullVersion;
    envConfig["created_at"] = getCurrentTimestamp();
    envConfig["description"] = "";

    config_["environments"][envName] = envConfig;
    saveConfig();

    Utils::printSuccess("Environment '" + envName + "' created successfully");
    Utils::printInfo("Location: " + envPath.string());

    return true;
}

bool PypManager::activateEnvironment(const std::string& envName) {
    if (!isValidEnvName(envName)) {
        Utils::printError("Invalid environment name: " + envName);
        return false;
    }

    fs::path envPath = resolveEnvironmentPath(envName);

    if (!fs::exists(envPath)) {
        Utils::printError("Environment '" + envName + "' does not exist");
        return false;
    }

    fs::path activateScript = envPath / "bin" / "activate";

    if (!fs::exists(activateScript)) {
        Utils::printError("Activation script not found in: " + activateScript.string());
        return false;
    }

    // Set the active environment variable
    setenv("PYP_CURRENT_ENV", envName.c_str(), 1);

    Utils::printSuccess("Environment '" + envName + "' activated");
    Utils::printInfo("To activate manually, run: source " + activateScript.string());
    Utils::printInfo("Or use: pyp run " + envName + " <command>");

    return true;
}

bool PypManager::deactivateEnvironment() {
    if (activeEnv_.empty()) {
        Utils::printWarning("No environment is currently active");
        return true;
    }

    unsetenv("PYP_CURRENT_ENV");
    Utils::printSuccess("Environment deactivated");
    activeEnv_ = "";

    return true;
}

std::vector<Environment> PypManager::listEnvironments() const {
    std::vector<Environment> environments;

    if (!config_.contains("environments") || !config_["environments"].is_object()) {
        return environments;
    }

    const char* activeEnv = getenv("PYP_CURRENT_ENV");
    std::string currentActive = activeEnv ? activeEnv : activeEnv_;

    std::string defaultEnv = config_.contains("settings") &&
                              config_["settings"].contains("default") ?
                              config_["settings"]["default"].get<std::string>() : "";

    for (const auto& [name, envConfig] : config_["environments"].items()) {
        Environment env;
        env.name = name;
        env.isActive = (name == currentActive);
        env.isDefault = (name == defaultEnv);

        if (envConfig.contains("path")) {
            env.path = fs::path(envConfig["path"].get<std::string>());
        } else {
            env.path = envsDir_ / name;
        }

        if (envConfig.contains("python_full_version")) {
            env.pythonVersion = envConfig["python_full_version"].get<std::string>();
        } else if (envConfig.contains("python")) {
            env.pythonVersion = envConfig["python"].get<std::string>();
        } else {
            env.pythonVersion = "unknown";
        }

        if (envConfig.contains("created_at")) {
            env.createdAt = envConfig["created_at"].get<std::string>();
        } else {
            env.createdAt = "unknown";
        }

        if (envConfig.contains("description")) {
            env.description = envConfig["description"].get<std::string>();
        } else {
            env.description = "";
        }

        // Check if environment directory exists
        env.isActive = env.isActive && fs::exists(env.path);

        environments.push_back(env);
    }

    return environments;
}

bool PypManager::removeEnvironment(const std::string& envName, bool force) {
    if (!isValidEnvName(envName)) {
        Utils::printError("Invalid environment name: " + envName);
        return false;
    }

    fs::path envPath = resolveEnvironmentPath(envName);

    if (!fs::exists(envPath)) {
        Utils::printError("Environment '" + envName + "' does not exist");
        return false;
    }

    // Check if environment is active
    const char* activeEnv = getenv("PYP_CURRENT_ENV");
    if (activeEnv && std::string(activeEnv) == envName) {
        Utils::printWarning("Cannot remove active environment. Deactivate it first.");
        return false;
    }

    // Confirmation
    if (!force) {
        Utils::printWarning("This will permanently remove: " + envName);
        Utils::printWarning("Environment path: " + envPath.string());

        std::cout << "Are you sure you want to remove this environment? [y/N] ";
        std::string confirmation;
        std::getline(std::cin, confirmation);

        if (confirmation != "y" && confirmation != "Y") {
            Utils::printInfo("Removal cancelled");
            return false;
        }
    }

    // Remove the environment directory
    if (!removeSecureDirectory(envPath)) {
        return false;
    }

    // Remove from configuration
    if (config_.contains("environments") && config_["environments"].contains(envName)) {
        config_["environments"].erase(envName);
    }

    // Update default if necessary
    if (config_.contains("settings") &&
        config_["settings"].contains("default") &&
        config_["settings"]["default"] == envName) {
        config_["settings"]["default"] = "";
    }

    saveConfig();

    Utils::printSuccess("Environment '" + envName + "' removed successfully");

    return true;
}

std::optional<Environment> PypManager::getEnvironmentInfo(const std::string& envName) const {
    auto environments = listEnvironments();
    for (const auto& env : environments) {
        if (env.name == envName) {
            return env;
        }
    }
    return std::nullopt;
}

bool PypManager::environmentExists(const std::string& envName) const {
    if (!isValidEnvName(envName)) {
        return false;
    }
    fs::path envPath = resolveEnvironmentPath(envName);
    return fs::exists(envPath);
}

std::optional<std::string> PypManager::getActiveEnvironment() const {
    const char* activeEnv = getenv("PYP_CURRENT_ENV");
    if (activeEnv && !std::string(activeEnv).empty()) {
        return std::string(activeEnv);
    }
    if (!activeEnv_.empty()) {
        return activeEnv_;
    }
    return std::nullopt;
}

fs::path PypManager::getPythonPath(const std::string& envName) const {
    fs::path envPath = resolveEnvironmentPath(envName);
    return envPath / "bin" / "python";
}

fs::path PypManager::getPipPath(const std::string& envName) const {
    fs::path envPath = resolveEnvironmentPath(envName);
    return envPath / "bin" / "pip";
}

int PypManager::runInEnvironment(const std::string& envName, const std::string& command) {
    fs::path envPath = resolveEnvironmentPath(envName);
    fs::path pythonPath = envPath / "bin" / "python";

    if (!fs::exists(pythonPath)) {
        Utils::printError("Python executable not found: " + pythonPath.string());
        return -1;
    }

    std::string fullCommand = Utils::escapeShellArg(pythonPath.string()) + " -c " + Utils::escapeShellArg(command);

    return system(fullCommand.c_str());
}

bool PypManager::exportEnvironment(const std::string& envName, const fs::path& outputPath) {
    if (!isValidEnvName(envName)) {
        Utils::printError("Invalid environment name: " + envName);
        return false;
    }

    fs::path envPath = resolveEnvironmentPath(envName);

    if (!fs::exists(envPath)) {
        Utils::printError("Environment '" + envName + "' does not exist");
        return false;
    }

    try {
        json exportData = json::object();
        exportData["exported_at"] = getCurrentTimestamp();
        exportData["environment_name"] = envName;
        exportData["environment_path"] = envPath.string();

        // Get list of installed packages
        std::string pipCommand = Utils::escapeShellArg(getPipPath(envName).string()) + " freeze";
        FILE* pipe = popen(pipCommand.c_str(), "r");
        if (pipe) {
            std::string packages;
            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                packages += buffer;
            }
            pclose(pipe);

            // Parse packages
            std::vector<std::string> packageList;
            std::istringstream ss(packages);
            std::string line;
            while (std::getline(ss, line)) {
                if (!line.empty()) {
                    packageList.push_back(line);
                }
            }
            exportData["packages"] = packageList;
        }

        // Write to file
        std::ofstream outFile(outputPath);
        if (!outFile.is_open()) {
            Utils::printError("Failed to create export file: " + outputPath.string());
            return false;
        }
        outFile << exportData.dump(2);
        outFile.close();

        Utils::printSuccess("Environment exported to: " + outputPath.string());
        return true;
    } catch (const std::exception& e) {
        Utils::printError("Export failed: " + std::string(e.what()));
        return false;
    }
}

bool PypManager::importEnvironment(const fs::path& importPath, const std::string& newName) {
    if (!fs::exists(importPath)) {
        Utils::printError("Import file not found: " + importPath.string());
        return false;
    }

    if (!isValidEnvName(newName)) {
        Utils::printError("Invalid environment name: " + newName);
        return false;
    }

    try {
        std::ifstream importFile(importPath);
        json importData = json::parse(importFile);
        importFile.close();

        // Create new environment
        if (!buildEnvironment(newName)) {
            return false;
        }

        // Install packages
        if (importData.contains("packages")) {
            fs::path pipPath = getPipPath(newName);
            std::string installCommand = Utils::escapeShellArg(pipPath.string()) + " install -r " +
                                         Utils::escapeShellArg(importPath.string());

            Utils::printInfo("Installing packages...");

            int exitCode = 0;
            if (!executeCommand(installCommand, exitCode)) {
                Utils::printWarning("Some packages may have failed to install");
            }
        }

        Utils::printSuccess("Environment imported as: " + newName);
        return true;
    } catch (const std::exception& e) {
        Utils::printError("Import failed: " + std::string(e.what()));
        return false;
    }
}

bool PypManager::setDefaultEnvironment(const std::string& envName) {
    if (!envName.empty() && !environmentExists(envName)) {
        Utils::printError("Environment '" + envName + "' does not exist");
        return false;
    }

    config_["settings"]["default"] = envName;
    saveConfig();

    if (envName.empty()) {
        Utils::printSuccess("Default environment cleared");
    } else {
        Utils::printSuccess("Default environment set to: " + envName);
    }

    return true;
}

std::string PypManager::getDefaultEnvironment() const {
    if (!config_.contains("settings") ||
        !config_["settings"].contains("default")) {
        return "";
    }
    return config_["settings"]["default"].get<std::string>();
}

bool PypManager::setEnvironmentDescription(const std::string& envName, const std::string& description) {
    if (!isValidEnvName(envName)) {
        Utils::printError("Invalid environment name: " + envName);
        return false;
    }

    if (!config_.contains("environments") ||
        !config_["environments"].contains(envName)) {
        Utils::printError("Environment '" + envName + "' not found in configuration");
        return false;
    }

    config_["environments"][envName]["description"] = description;
    saveConfig();

    Utils::printSuccess("Description updated for: " + envName);
    return true;
}

fs::path PypManager::getConfigDir() const {
    return configPath_.parent_path();
}

fs::path PypManager::getEnvsDir() const {
    return envsDir_;
}

int PypManager::cleanupOrphanedEnvironments() {
    if (!config_.contains("environments")) {
        return 0;
    }

    int cleanedCount = 0;
    std::vector<std::string> orphaned;

    for (const auto& [name, envConfig] : config_["environments"].items()) {
        fs::path envPath;
        if (envConfig.contains("path")) {
            envPath = fs::path(envConfig["path"].get<std::string>());
        } else {
            envPath = envsDir_ / name;
        }

        if (!fs::exists(envPath)) {
            orphaned.push_back(name);
        }
    }

    for (const auto& name : orphaned) {
        config_["environments"].erase(name);
        cleanedCount++;
        Utils::printInfo("Removed orphaned environment: " + name);
    }

    if (cleanedCount > 0) {
        saveConfig();
        Utils::printSuccess("Cleaned up " + std::to_string(cleanedCount) + " orphaned environment(s)");
    } else {
        Utils::printInfo("No orphaned environments found");
    }

    return cleanedCount;
}

json PypManager::environmentToJson(const Environment& env) const {
    json j = json::object();
    j["name"] = env.name;
    j["path"] = env.path.string();
    j["is_active"] = env.isActive;
    j["is_default"] = env.isDefault;
    j["python_version"] = env.pythonVersion;
    j["created_at"] = env.createdAt;
    j["description"] = env.description;
    return j;
}

Environment PypManager::jsonToEnvironment(const std::string& name, const json& j) const {
    Environment env;
    env.name = name;

    if (j.contains("path")) {
        env.path = fs::path(j["path"].get<std::string>());
    } else {
        env.path = envsDir_ / name;
    }

    env.isActive = j.value("is_active", false);
    env.isDefault = j.value("is_default", false);
    env.pythonVersion = j.value("python_version", "unknown");
    env.createdAt = j.value("created_at", "unknown");
    env.description = j.value("description", "");

    return env;
}

std::string PypManager::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::tm* tm = std::localtime(&time);
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
    
    return std::string(buffer);
}

bool PypManager::executeCommand(const std::string& command, int& exitCode) {
    exitCode = system(command.c_str());
    
    // Handle different return formats
    if (exitCode == 0) {
        return true;
    }
    
    // Check if it's a signal exit
    if (exitCode > 128) {
        // Killed by signal
        return false;
    }
    
    return false;
}

std::string PypManager::getShellActivationCommand(const fs::path& envPath) const {
    std::ostringstream oss;
    oss << "source " << Utils::escapeShellArg((envPath / "bin" / "activate").string());
    return oss.str();
}
