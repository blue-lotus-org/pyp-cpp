#include "CommandHandler.hpp"
#include "Utils.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

// Static command data
const std::map<std::string, std::vector<std::string>> CommandHandler::COMMAND_ALIASES = {
    {"build", {"create", "new", "make"}},
    {"activate", {"use", "enter", "source"}},
    {"deactivate", {"exit", "leave"}},
    {"list", {"ls", "show", "dir"}},
    {"remove", {"rm", "delete", "del"}},
    {"info", {"show", "details", "status"}},
    {"run", {"exec", "execute"}},
    {"export", {"save", "dump"}},
    {"import", {"load", "restore"}},
    {"set-default", {"default"}},
    {"cleanup", {"clean", "gc"}},
    {"init", {"initialize", "setup"}}
};

const std::map<std::string, std::string> CommandHandler::COMMAND_DESCRIPTIONS = {
    {"build", "Create a new virtual environment"},
    {"activate", "Activate a virtual environment"},
    {"deactivate", "Deactivate the current environment"},
    {"list", "List all virtual environments"},
    {"remove", "Remove a virtual environment"},
    {"info", "Show information about an environment"},
    {"run", "Run a command in an environment"},
    {"export", "Export environment to a file"},
    {"import", "Import environment from a file"},
    {"set-default", "Set the default environment"},
    {"cleanup", "Clean up orphaned environments"},
    {"init", "Initialize pyp configuration"}
};

const std::map<std::string, std::vector<std::string>> CommandHandler::COMMAND_USAGE = {
    {"build", {"<env_name> [--python VERSION] [--upgrade]"}},
    {"activate", {"<env_name>"}},
    {"deactivate", {""}},
    {"list", {"[--active] [--default]"}},
    {"remove", {"<env_name> [--force]"}},
    {"info", {"<env_name>"}},
    {"run", {"<env_name> <command>"}},
    {"export", {"<env_name> [--output FILE]"}},
    {"import", {"<file> [--name <env_name>]"}},
    {"set-default", {"[<env_name>]"}},
    {"cleanup", {""}},
    {"init", {""}}
};

CommandHandler::CommandHandler(PypManager& manager) : manager_(manager) {}

CommandHandler::~CommandHandler() {}

Command CommandHandler::parseArgs(int argc, char* argv[]) {
    Command cmd;
    
    if (argc < 2) {
        cmd.name = "help";
        return cmd;
    }
    
    cmd.name = argv[1];
    
    // Parse remaining arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        // Check for options (--something or --key=value)
        if (arg.length() > 2 && arg.substr(0, 2) == "--") {
            size_t pos = arg.find('=');
            if (pos != std::string::npos) {
                std::string key = arg.substr(2, pos - 2);
                std::string value = arg.substr(pos + 1);
                cmd.options[key] = value;
            } else {
                std::string key = arg.substr(2);
                cmd.options[key] = "true";
            }
        } else if (arg.length() > 1 && arg[0] == '-') {
            // Short options
            std::string shortOpt = arg.substr(1);
            for (char c : shortOpt) {
                std::string key(1, c);
                cmd.options[key] = "true";
            }
        } else {
            cmd.args.push_back(arg);
        }
    }
    
    return cmd;
}

std::string CommandHandler::resolveCommand(const std::string& command) const {
    std::string cmdLower = Utils::toLower(command);
    
    // Check if it's a direct command
    if (COMMAND_DESCRIPTIONS.find(cmdLower) != COMMAND_DESCRIPTIONS.end()) {
        return cmdLower;
    }
    
    // Check if it's an alias
    for (const auto& [mainCmd, aliases] : COMMAND_ALIASES) {
        for (const auto& alias : aliases) {
            if (Utils::toLower(alias) == cmdLower) {
                return mainCmd;
            }
        }
    }
    
    return cmdLower;
}

CommandResult CommandHandler::processArgs(int argc, char* argv[]) {
    Command cmd = parseArgs(argc, argv);
    cmd.name = resolveCommand(cmd.name);
    
    return executeCommand(cmd);
}

CommandResult CommandHandler::executeCommand(const Command& command) {
    // Handle help flag
    if (command.options.count("help") || command.options.count("h")) {
        if (command.name == "help" && !command.args.empty()) {
            showCommandHelp(command.args[0]);
            return CommandResult(true, 0);
        }
        showHelp();
        return CommandResult(true, 0);
    }
    
    // Version flag
    if (command.options.count("version") || command.options.count("v")) {
        showVersion();
        return CommandResult(true, 0);
    }
    
    // Execute the appropriate command
    if (command.name == "build" || command.name == "create") {
        return cmdBuild(command);
    } else if (command.name == "activate" || command.name == "use") {
        return cmdActivate(command);
    } else if (command.name == "deactivate") {
        return cmdDeactivate(command);
    } else if (command.name == "list" || command.name == "ls") {
        return cmdList(command);
    } else if (command.name == "remove" || command.name == "delete") {
        return cmdRemove(command);
    } else if (command.name == "info") {
        return cmdInfo(command);
    } else if (command.name == "run" || command.name == "exec") {
        return cmdRun(command);
    } else if (command.name == "export") {
        return cmdExport(command);
    } else if (command.name == "import") {
        return cmdImport(command);
    } else if (command.name == "set-default") {
        return cmdSetDefault(command);
    } else if (command.name == "cleanup") {
        return cmdCleanup(command);
    } else if (command.name == "init") {
        return cmdInit(command);
    } else if (command.name == "help") {
        showHelp();
        return CommandResult(true, 0);
    } else {
        Utils::printError("Unknown command: " + command.name);
        Utils::printInfo("Run 'pyp --help' for available commands");
        return CommandResult(false, 1, "Unknown command");
    }
}

CommandResult CommandHandler::cmdBuild(const Command& cmd) {
    if (!validateArgs("build", cmd, 1, -1)) {
        return CommandResult(false, 1, "Invalid arguments for build command");
    }
    
    std::string envName = cmd.args[0];
    std::string pythonVersion = "python3";
    bool upgrade = false;
    
    // Parse options
    if (cmd.options.count("python")) {
        pythonVersion = cmd.options.at("python");
    }
    if (cmd.options.count("p")) {
        pythonVersion = cmd.options.at("p");
    }
    if (cmd.options.count("upgrade") || cmd.options.count("U")) {
        upgrade = true;
    }
    
    bool success = manager_.buildEnvironment(envName, pythonVersion, upgrade);
    return CommandResult(success, success ? 0 : 1);
}

CommandResult CommandHandler::cmdActivate(const Command& cmd) {
    if (!validateArgs("activate", cmd, 1, 1)) {
        return CommandResult(false, 1, "Invalid arguments for activate command");
    }
    
    std::string envName = cmd.args[0];
    bool success = manager_.activateEnvironment(envName);
    return CommandResult(success, success ? 0 : 1);
}

CommandResult CommandHandler::cmdDeactivate(const Command& cmd) {
    (void)cmd;
    bool success = manager_.deactivateEnvironment();
    return CommandResult(success, success ? 0 : 1);
}

CommandResult CommandHandler::cmdList(const Command& cmd) {
    (void)cmd;
    std::vector<Environment> environments = manager_.listEnvironments();
    
    if (environments.empty()) {
        Utils::printInfo("No environments found. Create one with 'pyp build <name>'");
        return CommandResult(true, 0);
    }
    
    // Filter if requested
    bool showActiveOnly = cmd.options.count("active");
    bool showDefaultOnly = cmd.options.count("default");
    
    if (showActiveOnly) {
        std::vector<Environment> filtered;
        for (const auto& env : environments) {
            if (env.isActive) {
                filtered.push_back(env);
            }
        }
        environments = filtered;
    }
    
    if (showDefaultOnly) {
        std::vector<Environment> filtered;
        for (const auto& env : environments) {
            if (env.isDefault) {
                filtered.push_back(env);
            }
        }
        environments = filtered;
    }
    
    printEnvironmentTable(environments);
    return CommandResult(true, 0);
}

CommandResult CommandHandler::cmdRemove(const Command& cmd) {
    if (!validateArgs("remove", cmd, 1, 1)) {
        return CommandResult(false, 1, "Invalid arguments for remove command");
    }
    
    std::string envName = cmd.args[0];
    bool force = cmd.options.count("force") || cmd.options.count("f");
    
    bool success = manager_.removeEnvironment(envName, force);
    return CommandResult(success, success ? 0 : 1);
}

CommandResult CommandHandler::cmdInfo(const Command& cmd) {
    if (!validateArgs("info", cmd, 1, 1)) {
        return CommandResult(false, 1, "Invalid arguments for info command");
    }
    
    std::string envName = cmd.args[0];
    auto envInfo = manager_.getEnvironmentInfo(envName);
    
    if (!envInfo.has_value()) {
        Utils::printError("Environment '" + envName + "' not found");
        return CommandResult(false, 1, "Environment not found");
    }
    
    Environment env = envInfo.value();
    
    Utils::printHeader("Environment Information");
    std::cout << std::left;
    std::cout << std::setw(20) << "Name:" << env.name << std::endl;
    std::cout << std::setw(20) << "Path:" << env.path.string() << std::endl;
    std::cout << std::setw(20) << "Python:" << env.pythonVersion << std::endl;
    std::cout << std::setw(20) << "Created:" << env.createdAt << std::endl;
    std::cout << std::setw(20) << "Active:" << (env.isActive ? "Yes" : "No") << std::endl;
    std::cout << std::setw(20) << "Default:" << (env.isDefault ? "Yes" : "No") << std::endl;
    
    if (!env.description.empty()) {
        std::cout << std::setw(20) << "Description:" << env.description << std::endl;
    }
    
    return CommandResult(true, 0);
}

CommandResult CommandHandler::cmdRun(const Command& cmd) {
    if (!validateArgs("run", cmd, 2, -1)) {
        return CommandResult(false, 1, "Invalid arguments for run command");
    }
    
    std::string envName = cmd.args[0];
    std::string command = Utils::join(
        std::vector<std::string>(cmd.args.begin() + 1, cmd.args.end()),
        " "
    );
    
    int exitCode = manager_.runInEnvironment(envName, command);
    return CommandResult(exitCode == 0, exitCode);
}

CommandResult CommandHandler::cmdExport(const Command& cmd) {
    if (!validateArgs("export", cmd, 1, 1)) {
        return CommandResult(false, 1, "Invalid arguments for export command");
    }
    
    std::string envName = cmd.args[0];
    fs::path outputPath;
    
    if (cmd.options.count("output") || cmd.options.count("o")) {
        std::string option = cmd.options.count("output") ? "output" : "o";
        outputPath = fs::path(cmd.options.at(option));
    } else {
        outputPath = fs::path(envName + "_export.json");
    }
    
    bool success = manager_.exportEnvironment(envName, outputPath);
    return CommandResult(success, success ? 0 : 1);
}

CommandResult CommandHandler::cmdImport(const Command& cmd) {
    if (!validateArgs("import", cmd, 1, 1)) {
        return CommandResult(false, 1, "Invalid arguments for import command");
    }
    
    fs::path importPath = fs::path(cmd.args[0]);
    std::string newName = importPath.stem().string();
    
    if (cmd.options.count("name") || cmd.options.count("n")) {
        std::string option = cmd.options.count("name") ? "name" : "n";
        newName = cmd.options.at(option);
    }
    
    bool success = manager_.importEnvironment(importPath, newName);
    return CommandResult(success, success ? 0 : 1);
}

CommandResult CommandHandler::cmdSetDefault(const Command& cmd) {
    std::string envName;
    
    if (!cmd.args.empty()) {
        envName = cmd.args[0];
    }
    
    bool success = manager_.setDefaultEnvironment(envName);
    return CommandResult(success, success ? 0 : 1);
}

CommandResult CommandHandler::cmdCleanup(const Command& cmd) {
    (void)cmd;
    int cleaned = manager_.cleanupOrphanedEnvironments();
    return CommandResult(true, 0);
}

CommandResult CommandHandler::cmdInit(const Command& cmd) {
    (void)cmd;
    bool success = manager_.initialize();
    return CommandResult(success, success ? 0 : 1);
}

void CommandHandler::printEnvironmentTable(const std::vector<Environment>& environments) const {
    std::cout << std::endl;
    std::cout << std::left << std::setw(25) << "NAME"
              << std::setw(20) << "STATUS"
              << std::setw(15) << "PYTHON"
              << "CREATED" << std::endl;
    std::cout << std::string(75, '-') << std::endl;
    
    for (const auto& env : environments) {
        std::string status;
        if (env.isActive) {
            status = Utils::GREEN + "[ACTIVE]" + Utils::RESET;
        } else if (env.isDefault) {
            status = Utils::YELLOW + "[DEFAULT]" + Utils::RESET;
        } else {
            status = "";
        }
        
        std::cout << std::left << std::setw(25) << env.name
                  << std::setw(20) << status
                  << std::setw(15) << env.pythonVersion
                  << env.createdAt << std::endl;
    }
    
    std::cout << std::endl;
}

std::string CommandHandler::formatEnvironment(const Environment& env) const {
    std::ostringstream oss;
    oss << env.name;
    
    if (env.isActive) {
        oss << Utils::GREEN << " *" << Utils::RESET;
    } else if (env.isDefault) {
        oss << Utils::YELLOW << " [default]" << Utils::RESET;
    }
    
    return oss.str();
}

void CommandHandler::showHelp() const {
    std::cout << std::endl;
    std::cout << Utils::BOLD << Utils::CYAN;
    std::cout << "  pyp - Secure Python Virtual Environment Manager  " << std::endl;
    std::cout << Utils::RESET << std::endl;
    std::cout << std::endl;
    
    std::cout << Utils::BOLD << "USAGE:" << Utils::RESET << std::endl;
    std::cout << "    pyp [COMMAND] [OPTIONS] [ARGS]" << std::endl;
    std::cout << std::endl;
    
    std::cout << Utils::BOLD << "COMMANDS:" << Utils::RESET << std::endl;
    
    // Calculate max command name length for alignment
    size_t maxLen = 0;
    for (const auto& [cmd, desc] : COMMAND_DESCRIPTIONS) {
        maxLen = std::max(maxLen, cmd.length());
    }
    
    for (const auto& [cmd, desc] : COMMAND_DESCRIPTIONS) {
        std::cout << "    " << std::left << std::setw(15) << cmd << desc << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << Utils::BOLD << "OPTIONS:" << Utils::RESET << std::endl;
    std::cout << "    -h, --help     Show this help message" << std::endl;
    std::cout << "    -v, --version  Show version information" << std::endl;
    std::cout << std::endl;
    
    std::cout << Utils::BOLD << "EXAMPLES:" << Utils::RESET << std::endl;
    std::cout << "    pyp build myenv              Create a new environment" << std::endl;
    std::cout << "    pyp activate myenv           Activate an environment" << std::endl;
    std::cout << "    pyp list                     List all environments" << std::endl;
    std::cout << "    pyp run myenv python --version" << std::endl;
    std::cout << "    pyp remove myenv             Remove an environment" << std::endl;
    std::cout << std::endl;
    
    std::cout << "For more information about a command, run:" << std::endl;
    std::cout << "    pyp help <command>" << std::endl;
    std::cout << std::endl;
}

void CommandHandler::showVersion() const {
    std::cout << std::endl;
    std::cout << Utils::BOLD << Utils::CYAN << "pyp" << Utils::RESET << std::endl;
    std::cout << "Version: 1.0.0" << std::endl;
    std::cout << "A secure Python virtual environment manager written in C++" << std::endl;
    std::cout << std::endl;
}

void CommandHandler::showUsage() const {
    std::cout << "Usage: pyp <command> [options] [arguments]" << std::endl;
    std::cout << "Run 'pyp --help' for available commands" << std::endl;
}

void CommandHandler::showCommandHelp(const std::string& command) const {
    std::string cmd = Utils::toLower(command);
    cmd = resolveCommand(cmd);
    
    if (COMMAND_DESCRIPTIONS.find(cmd) == COMMAND_DESCRIPTIONS.end()) {
        Utils::printError("Unknown command: " + command);
        return;
    }
    
    std::cout << std::endl;
    std::cout << Utils::BOLD << "pyp " << cmd << Utils::RESET << std::endl;
    std::cout << std::endl;
    std::cout << COMMAND_DESCRIPTIONS.at(cmd) << std::endl;
    std::cout << std::endl;
    
    if (COMMAND_USAGE.find(cmd) != COMMAND_USAGE.end()) {
        std::cout << Utils::BOLD << "USAGE:" << Utils::RESET << std::endl;
        for (const auto& usage : COMMAND_USAGE.at(cmd)) {
            std::cout << "    pyp " << cmd << " " << usage << std::endl;
        }
        std::cout << std::endl;
    }
    
    std::cout << Utils::BOLD << "ALIASES:" << Utils::RESET << std::endl;
    bool hasAliases = false;
    for (const auto& [mainCmd, aliases] : COMMAND_ALIASES) {
        if (mainCmd == cmd) {
            for (const auto& alias : aliases) {
                std::cout << "    " << alias << std::endl;
                hasAliases = true;
            }
        }
    }
    if (!hasAliases) {
        std::cout << "    (none)" << std::endl;
    }
    std::cout << std::endl;
}

bool CommandHandler::validateArgs(const std::string& command, const Command& cmd, int minArgs, int maxArgs) const {
    size_t actualArgs = cmd.args.size();
    
    if (minArgs >= 0 && actualArgs < static_cast<size_t>(minArgs)) {
        Utils::printError("Missing required argument for " + command);
        showCommandHelp(command);
        return false;
    }
    
    if (maxArgs >= 0 && actualArgs > static_cast<size_t>(maxArgs)) {
        Utils::printError("Too many arguments for " + command);
        showCommandHelp(command);
        return false;
    }
    
    return true;
}
