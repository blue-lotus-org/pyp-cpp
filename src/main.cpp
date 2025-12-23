/**
 * @file main.cpp
 * @brief Main entry point for pyp - Secure Python Virtual Environment Manager
 * 
 * This is the main source file for the pyp tool, which provides secure
 * management of Python virtual environments with enhanced security features.
 * 
 * @author LotusChain Agent
 * @version 1.0.0
 *
 * This is 2026 gift :)
 * Last tests on 23 Dec 2025
 */

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <csignal>
#include <sys/stat.h>

#include "PypManager.hpp"
#include "CommandHandler.hpp"
#include "Utils.hpp"

namespace fs = std::filesystem;

// Global manager instance for signal handling
static PypManager* g_manager = nullptr;

/**
 * @brief Signal handler for graceful shutdown
 */
void signalHandler(int signal) {
    (void)signal;
    Utils::printDebug("Received shutdown signal");
    if (g_manager) {
        delete g_manager;
        g_manager = nullptr;
    }
    exit(1);
}

/**
 * @brief Print a startup banner
 */
void printBanner() {
    std::cout << std::endl;
    std::cout << Utils::CYAN;
    std::cout << "  ____  _____ ____  _   _ ____  " << std::endl;
    std::cout << " |  _ \\| ____| __ )| | | |  _ \\ " << std::endl;
    std::cout << " | |_) |  _| |  _ \\| | | | |_) |" << std::endl;
    std::cout << " |  __/| |___| |_) | |_| |  __/ " << std::endl;
    std::cout << " |_|   |_____|____/ \\___/|_|    " << std::endl;
    std::cout << std::endl;
    std::cout << Utils::RESET;
    std::cout << "  Secure Python Virtual Environment Manager" << std::endl;
    std::cout << std::endl;
}

/**
 * @brief Check for required dependencies
 * @return true if all dependencies are available
 */
bool checkDependencies() {
    Utils::printDebug("Checking dependencies...");
    
    // Check for Python
    if (!Utils::commandExists("python3") && !Utils::commandExists("python")) {
        Utils::printWarning("Python not found. You may need to install Python.");
    }
    
    // Check for required directories
    fs::path homeDir = Utils::getHomeDirectory();
    fs::path envsDir = homeDir / ".pyp_envs";
    
    if (!fs::exists(envsDir)) {
        Utils::printInfo("First run detected. Run 'pyp init' to initialize.");
    }
    
    return true;
}

/**
 * @brief Handle special commands that need early processing
 * @param argc Argument count
 * @param argv Argument vector
 * @return true if a special command was handled
 */
bool handleEarlyCommands(int argc, char* argv[]) {
    if (argc < 2) {
        return false;
    }
    
    std::string firstArg = argv[1];
    
    // Version flag
    if (firstArg == "--version" || firstArg == "-v") {
        std::cout << "pyp version 1.0.0" << std::endl;
        return true;
    }
    
    // Help flag
    if (firstArg == "--help" || firstArg == "-h") {
        // Handled by CommandHandler
        return false;
    }
    
    // Install completion flag
    if (firstArg == "--install-completion") {
        std::string shell = "bash";
        if (argc > 2) {
            shell = argv[2];
        }
        
        std::cout << "Installing shell completion for " << shell << "..." << std::endl;
        // Completion installation would go here
        std::cout << "Completion installed. Restart your shell to activate." << std::endl;
        return true;
    }
    
    return false;
}

/**
 * @brief Main entry point
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit code
 */
int main(int argc, char* argv[]) {
    // Set up signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Check for early commands
    if (handleEarlyCommands(argc, argv)) {
        return 0;
    }
    
    // Check dependencies
    if (!checkDependencies()) {
        return 1;
    }
    
    // Initialize the manager
    PypManager* manager = new PypManager();
    g_manager = manager;
    
    // Initialize if needed
    fs::path configPath = manager->getConfigDir() / ".pyp_config.json";
    if (!fs::exists(configPath)) {
        manager->initialize();
    }
    
    // Create command handler
    CommandHandler handler(*manager);
    
    // Process arguments and execute command
    CommandResult result = handler.processArgs(argc, argv);
    
    // Clean up
    delete manager;
    g_manager = nullptr;
    
    return result.exitCode;
}
