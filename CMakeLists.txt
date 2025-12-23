cmake_minimum_required(VERSION 3.14)
project(pyp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch nlohmann/json for JSON handling
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)

# Find required packages
find_package(Boost REQUIRED COMPONENTS filesystem system)
find_package(CURL REQUIRED)

# Main executable
add_executable(pyp
    src/main.cpp
    src/PypManager.cpp
    src/Utils.cpp
    src/CommandHandler.cpp
)

target_include_directories(pyp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(pyp PRIVATE
    nlohmann::json
    Boost::filesystem
    Boost::system
    CURL::libcurl
)

# Enable address sanitizer for debugging (optional)
# target_link_options(pyp PRIVATE -fsanitize=address)

# Installation configuration
include(GNUInstallDirs)

install(TARGETS pyp
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES data/pyp_completion.bash
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/bash-completion/completions
)

install(FILES data/pyp_completion.zsh
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/zsh/site-functions
)

# Create config directory
install(CODE "
    file(MAKE_DIRECTORY \$ENV{DESTDIR}/${CMAKE_INSTALL_FULL_SYSCONFDIR}/pyp)
    file(MAKE_DIRECTORY \$ENV{DESTDIR}/${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/pyp/envs)
")

message(STATUS "pyp will be installed to: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Binary: ${CMAKE_INSTALL_BINDIR}/pyp")
