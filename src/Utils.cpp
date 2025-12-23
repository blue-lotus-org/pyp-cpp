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
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <thread>
#include <regex>

// Color codes
const std::string Utils::RESET = "\033[0m";
const std::string Utils::RED = "\033[31m";
const std::string Utils::GREEN = "\033[32m";
const std::string Utils::YELLOW = "\033[33m";
const std::string Utils::BLUE = "\033[34m";
const std::string Utils::MAGENTA = "\033[35m";
const std::string Utils::CYAN = "\033[36m";
const std::string Utils::WHITE = "\033[37m";
const std::string Utils::BOLD = "\033[1m";

// ==================== Output Functions ====================

void Utils::printSuccess(const std::string& message) {
    std::cout << GREEN << "[+] " << RESET << message << std::endl;
}

void Utils::printError(const std::string& message) {
    std::cerr << RED << "[-] " << RESET << message << std::endl;
}

void Utils::printWarning(const std::string& message) {
    std::cout << YELLOW << "[!] " << RESET << message << std::endl;
}

void Utils::printInfo(const std::string& message) {
    std::cout << BLUE << "[*] " << RESET << message << std::endl;
}

void Utils::printDebug(const std::string& message) {
#ifdef DEBUG
    std::cout << CYAN << "[D] " << RESET << message << std::endl;
#else
    (void)message; // Suppress unused parameter warning in release builds
#endif
}

void Utils::printHeader(const std::string& title) {
    std::cout << std::endl;
    std::cout << BOLD << CYAN << "=== " << title << " ===" << RESET << std::endl;
    std::cout << std::endl;
}

// ==================== String Functions ====================

std::string Utils::escapeShellArg(const std::string& input) {
    std::string result;
    result.reserve(input.size() + 2);
    
    result += "'";
    
    for (char c : input) {
        if (c == '\'') {
            result += "'\\''";
        } else {
            result += c;
        }
    }
    
    result += "'";
    
    return result;
}

std::string Utils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

std::string Utils::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string Utils::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

bool Utils::startsWith(const std::string& str, const std::string& prefix) {
    if (str.length() < prefix.length()) {
        return false;
    }
    return str.compare(0, prefix.length(), prefix) == 0;
}

bool Utils::endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::vector<std::string> Utils::split(const std::string& str, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream ss(str);
    std::string part;
    
    while (std::getline(ss, part, delimiter)) {
        parts.push_back(part);
    }
    
    return parts;
}

std::string Utils::join(const std::vector<std::string>& parts, const std::string& separator) {
    if (parts.empty()) {
        return "";
    }
    
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        result += parts[i];
        if (i < parts.size() - 1) {
            result += separator;
        }
    }
    
    return result;
}

// ==================== File Functions ====================

bool Utils::fileExists(const fs::path& path) {
    struct stat buffer;
    return stat(path.string().c_str(), &buffer) == 0;
}

bool Utils::isDirectory(const fs::path& path) {
    struct stat buffer;
    if (stat(path.string().c_str(), &buffer) != 0) {
        return false;
    }
    return S_ISDIR(buffer.st_mode);
}

std::string Utils::readFile(const fs::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool Utils::writeFile(const fs::path& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    file.close();
    
    return true;
}

long long Utils::getFileSize(const fs::path& path) {
    struct stat buffer;
    if (stat(path.string().c_str(), &buffer) != 0) {
        return -1;
    }
    return buffer.st_size;
}

fs::path Utils::createTempFile(const std::string& prefix) {
    std::string tempPath = "/tmp/" + prefix + "_XXXXXX";
    char* buffer = new char[tempPath.size() + 1];
    std::strcpy(buffer, tempPath.c_str());
    
    int fd = mkstemp(buffer);
    if (fd == -1) {
        delete[] buffer;
        return fs::path();
    }
    
    close(fd);
    fs::path result(buffer);
    delete[] buffer;
    
    return result;
}

// ==================== System Functions ====================

fs::path Utils::getHomeDirectory() {
    const char* homeDir = getenv("HOME");
    if (homeDir) {
        return fs::path(homeDir);
    }
    
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        return fs::path(pw->pw_dir);
    }
    
    return fs::path("/tmp");
}

fs::path Utils::getCurrentDirectory() {
    char buffer[4096];
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return fs::path(buffer);
    }
    return fs::path("/");
}

std::string Utils::getUsername() {
    const char* user = getenv("USER");
    if (user) {
        return std::string(user);
    }
    
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_name) {
        return std::string(pw->pw_name);
    }
    
    return "unknown";
}

int Utils::executeCommand(const std::string& command, std::string& output) {
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return -1;
    }
    
    char buffer[1024];
    output.clear();
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    
    int status = pclose(pipe);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    
    return -1;
}

int Utils::executeCommand(const std::string& command) {
    int result = system(command.c_str());
    
    if (result == 0) {
        return 0;
    }
    
    if (result > 128) {
        return result;
    }
    
    return -1;
}

bool Utils::commandExists(const std::string& command) {
    std::string whichCommand = "command -v " + escapeShellArg(command) + " > /dev/null 2>&1";
    int result = system(whichCommand.c_str());
    return result == 0;
}

std::string Utils::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::tm* tm = std::localtime(&time);
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
    
    return std::string(buffer);
}

void Utils::sleep(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

// ==================== Validation Functions ====================

bool Utils::isValidEnvName(const std::string& name) {
    if (name.empty() || name.length() > 64) {
        return false;
    }
    
    for (char c : name) {
        if (!std::isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }
    
    if (name == "." || name == ".." || name[0] == '.') {
        return false;
    }
    
    return true;
}

bool Utils::isValidPath(const std::string& path) {
    if (path.empty() || path.length() > 4096) {
        return false;
    }
    
    // Check for null bytes
    if (path.find('\0') != std::string::npos) {
        return false;
    }
    
    // Check for path traversal attempts
    if (path.find("..") != std::string::npos) {
        return false;
    }
    
    return true;
}

// ==================== Format Functions ====================

std::string Utils::formatSize(long long bytes) {
    if (bytes < 0) {
        return "unknown";
    }
    
    if (bytes < 1024) {
        return std::to_string(bytes) + " B";
    }
    
    if (bytes < 1024 * 1024) {
        return std::to_string(bytes / 1024) + " KB";
    }
    
    if (bytes < 1024 * 1024 * 1024) {
        return std::to_string(bytes / (1024 * 1024)) + " MB";
    }
    
    return std::to_string(bytes / (1024 * 1024 * 1024)) + " GB";
}

std::string Utils::formatDuration(long long milliseconds) {
    if (milliseconds < 0) {
        return "unknown";
    }
    
    if (milliseconds < 1000) {
        return std::to_string(milliseconds) + " ms";
    }
    
    if (milliseconds < 60 * 1000) {
        return std::to_string(milliseconds / 1000) + " s";
    }
    
    if (milliseconds < 60 * 60 * 1000) {
        return std::to_string(milliseconds / (60 * 1000)) + " min";
    }
    
    return std::to_string(milliseconds / (60 * 60 * 1000)) + " h";
}
