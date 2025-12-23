#!/bin/bash
#
# pyp C++ - Install Script
# Secure Python Virtual Environment Manager
#
# This script installs pyp (C++ version) on your system.
#
# Usage:
#   ./install.sh [options]
#
# Options:
#   --prefix=PATH     Installation prefix (default: /usr/local)
#   --bindir=PATH     Binary directory (default: bin)
#   --confdir=PATH    Configuration directory (default: etc)
#   --local           Install to $HOME instead of system directories
#   --uninstall       Remove the installed files
#   --help            Show this help message
#
# This is 2026 gift :)
# Last tests on 23 Dec 2025
# https://lotuschain.org -> Startup Studio HUB

set -e

# Default installation paths
PREFIX="${PREFIX:-/usr/local}"
BINDIR="${BINDIR:-bin}"
CONFDIR="${CONFDIR:-etc}"
SYSCONFDIR="${SYSCONFDIR:-$PREFIX/$CONFDIR}"
LOCALSTATEDIR="${LOCALSTATEDIR:-$PREFIX/var}"
DATADIR="${DATADIR:-$PREFIX/share}"

# Installation directories
INSTALL_BINDIR="$PREFIX/$BINDIR"
INSTALL_SYSCONFDIR="$SYSCONFDIR/pyp"
INSTALL_LOCALSTATEDIR="$LOCALSTATEDIR/lib/pyp/envs"
INSTALL_DATADIR="$DATADIR/bash-completion/completions"
INSTALL_ZSHDIR="$DATADIR/zsh/site-functions"

# Binary name
BINARY_NAME="pyp"

# Source directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print functions
print_info() {
    echo -e "${BLUE}[*]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[+]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

print_error() {
    echo -e "${RED}[-]${NC} $1"
}

# Show help
show_help() {
    cat << EOF
pyp C++ - Install Script

Usage: $0 [options]

Options:
  --prefix=PATH     Installation prefix (default: /usr/local)
  --bindir=PATH     Binary directory (default: bin)
  --confdir=PATH    Configuration directory (default: etc)
  --local           Install to \$HOME instead of system directories
  --uninstall       Remove the installed files
  --help            Show this help message

Examples:
  $0                          # Install to /usr/local
  $0 --local                  # Install to \$HOME
  $0 --prefix=\$HOME/.local   # Install to custom prefix
  $0 --uninstall              # Uninstall

EOF
}

# Parse command line arguments
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --prefix=*)
                PREFIX="${1#*=}"
                shift
                ;;
            --bindir=*)
                BINDIR="${1#*=}"
                shift
                ;;
            --confdir=*)
                CONFDIR="${1#*=}"
                shift
                ;;
            --local)
                PREFIX="$HOME/.local"
                shift
                ;;
            --uninstall)
                UNINSTALL=1
                shift
                ;;
            --help)
                show_help
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

# Check if running as root
check_root() {
    if [[ $EUID -eq 0 ]]; then
        SYSTEM_INSTALL=1
        print_info "Running as root - will install system-wide"
    else
        SYSTEM_INSTALL=0
        print_info "Running as regular user - will install to user directory"
    fi
}

# Check dependencies
check_dependencies() {
    print_info "Checking dependencies..."
    
    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake is required but not installed"
        print_info "Install cmake: apt-get install cmake (Debian/Ubuntu)"
        print_info "              yum install cmake (RHEL/CentOS)"
        exit 1
    fi
    
    # Check for C++ compiler
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        print_error "C++ compiler is required but not installed"
        print_info "Install g++: apt-get install g++ (Debian/Ubuntu)"
        print_info "             yum install gcc-c++ (RHEL/CentOS)"
        exit 1
    fi
    
    # Check for Boost
    if ! ldconfig -p 2>/dev/null | grep -q libboost_filesystem; then
        print_warning "Boost libraries not found. Will try to build anyway."
    fi
    
    print_success "All dependencies satisfied"
}

# Build the project
build_project() {
    print_info "Building pyp C++..."
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Run CMake
    print_info "Running CMake..."
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DCMAKE_INSTALL_BINDIR="$BINDIR" \
        -DCMAKE_INSTALL_SYSCONFDIR="$CONFDIR"
    
    # Build
    print_info "Compiling..."
    make -j$(nproc)
    
    if [[ ! -f "$BUILD_DIR/pyp" ]]; then
        print_error "Build failed - pyp binary not found"
        exit 1
    fi
    
    print_success "Build completed successfully"
}

# Install the binary
install_binary() {
    print_info "Installing binary..."
    
    # Create directories
    mkdir -p "$INSTALL_BINDIR"
    mkdir -p "$INSTALL_SYSCONFDIR"
    mkdir -p "$INSTALL_LOCALSTATEDIR"
    mkdir -p "$INSTALL_DATADIR"
    mkdir -p "$INSTALL_ZSHDIR"
    
    # Install binary
    cp "$BUILD_DIR/pyp" "$INSTALL_BINDIR/"
    chmod 755 "$INSTALL_BINDIR/pyp"
    
    # Set secure permissions on config directory
    chmod 700 "$INSTALL_SYSCONFDIR"
    
    print_success "Binary installed to: $INSTALL_BINDIR/pyp"
}

# Install shell completion
install_completion() {
    print_info "Installing shell completion..."
    
    # Install bash completion
    if [[ -f "$SCRIPT_DIR/data/pyp_completion.bash" ]]; then
        cp "$SCRIPT_DIR/data/pyp_completion.bash" "$INSTALL_DATADIR/pyp"
        chmod 644 "$INSTALL_DATADIR/pyp"
        print_success "Bash completion installed to: $INSTALL_DATADIR/pyp"
    fi
    
    # Install zsh completion
    if [[ -f "$SCRIPT_DIR/data/pyp_completion.zsh" ]]; then
        cp "$SCRIPT_DIR/data/pyp_completion.zsh" "$INSTALL_ZSHDIR/_pyp"
        chmod 644 "$INSTALL_ZSHDIR/_pyp"
        print_success "Zsh completion installed to: $INSTALL_ZSHDIR/_pyp"
    fi
}

# Create wrapper script
create_wrapper() {
    print_info "Creating wrapper script..."
    
    # Create a wrapper that sets up the environment
    cat > "$INSTALL_BINDIR/pyp" << 'WRAPPER'
#!/bin/bash
#
# pyp wrapper script
# Sets up environment and launches the pyp binary
#

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set up environment variables
export PYP_HOME="${HOME}/.pyp"
export PYP_ENVS_DIR="${PYP_HOME}/envs"
export PYP_CONFIG_FILE="${PYP_HOME}/config.json"

# Create directories if they don't exist
mkdir -p "${PYP_ENVS_DIR}"
mkdir -p "$(dirname "${PYP_CONFIG_FILE}")"

# Launch the actual binary
exec "${SCRIPT_DIR}/pyp.bin" "$@"
WRAPPER

    chmod 755 "$INSTALL_BINDIR/pyp"
    
    # Also install the actual binary with a different name
    if [[ -f "$BUILD_DIR/pyp" ]]; then
        cp "$BUILD_DIR/pyp" "$INSTALL_BINDIR/pyp.bin"
        chmod 755 "$INSTALL_BINDIR/pyp.bin"
    fi
    
    print_success "Wrapper script created"
}

# Create symlink
create_symlink() {
    print_info "Creating symlink..."
    
    # Try to create symlink in /usr/local/bin
    if [[ -w /usr/local/bin ]] || [[ $EUID -eq 0 ]]; then
        ln -sf "$INSTALL_BINDIR/pyp" /usr/local/bin/pyp 2>/dev/null || true
        print_success "Symlink created at /usr/local/bin/pyp"
    fi
    
    # Also try to create in ~/bin if it exists
    if [[ -d "$HOME/bin" ]]; then
        ln -sf "$INSTALL_BINDIR/pyp" "$HOME/bin/pyp" 2>/dev/null || true
    fi
}

# Create environment directory
setup_environment() {
    print_info "Setting up pyp environment..."
    
    # Create user environment directory
    ENV_DIR="${HOME}/.pyp/envs"
    mkdir -p "$ENV_DIR"
    
    # Set secure permissions
    chmod 700 "$ENV_DIR"
    
    # Create default config
    CONFIG_FILE="${HOME}/.pyp/config.json"
    if [[ ! -f "$CONFIG_FILE" ]]; then
        cat > "$CONFIG_FILE" << 'CONFIG'
{
  "version": "1.0.0",
  "environments": {},
  "settings": {
    "default": ""
  }
}
CONFIG
        chmod 600 "$CONFIG_FILE"
    fi
    
    print_success "Environment setup complete"
    print_info "Environment directory: $ENV_DIR"
    print_info "Config file: $CONFIG_FILE"
}

# Run installation tests
run_tests() {
    print_info "Running installation tests..."
    
    # Test binary
    if [[ -x "$INSTALL_BINDIR/pyp" ]]; then
        print_success "Binary is executable"
    else
        print_error "Binary is not executable"
        return 1
    fi
    
    # Test version
    if "$INSTALL_BINDIR/pyp" --version > /dev/null 2>&1; then
        print_success "Binary runs correctly"
    else
        print_error "Binary failed to run"
        return 1
    fi
    
    # Test help
    if "$INSTALL_BINDIR/pyp" --help > /dev/null 2>&1; then
        print_success "Help command works"
    else
        print_error "Help command failed"
        return 1
    fi
    
    print_success "All tests passed"
    return 0
}

# Uninstall
uninstall() {
    print_warning "Uninstalling pyp..."
    
    # Remove binary
    if [[ -f "$INSTALL_BINDIR/pyp" ]]; then
        rm -f "$INSTALL_BINDIR/pyp"
        print_info "Removed: $INSTALL_BINDIR/pyp"
    fi
    
    if [[ -f "$INSTALL_BINDIR/pyp.bin" ]]; then
        rm -f "$INSTALL_BINDIR/pyp.bin"
        print_info "Removed: $INSTALL_BINDIR/pyp.bin"
    fi
    
    # Remove symlink
    if [[ -L /usr/local/bin/pyp ]]; then
        rm -f /usr/local/bin/pyp
        print_info "Removed symlink: /usr/local/bin/pyp"
    fi
    
    # Remove completion files
    if [[ -f "$INSTALL_DATADIR/pyp" ]]; then
        rm -f "$INSTALL_DATADIR/pyp"
        print_info "Removed: $INSTALL_DATADIR/pyp"
    fi
    
    if [[ -f "$INSTALL_ZSHDIR/_pyp" ]]; then
        rm -f "$INSTALL_ZSHDIR/_pyp"
        print_info "Removed: $INSTALL_ZSHDIR/_pyp"
    fi
    
    # Ask about user data
    echo ""
    read -p "Remove user data (~/.pyp)? [y/N] " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -rf "${HOME}/.pyp"
        print_info "Removed user data: ${HOME}/.pyp"
    fi
    
    print_success "Uninstallation complete"
}

# Print installation summary
print_summary() {
    echo ""
    echo "========================================"
    print_success "Installation Complete!"
    echo "========================================"
    echo ""
    echo "Binary:    $INSTALL_BINDIR/pyp"
    echo "Symlink:   /usr/local/bin/pyp"
    echo ""
    echo "Quick start:"
    echo "  pyp --help              Show help"
    echo "  pyp init                Initialize pyp"
    echo "  pyp build myenv         Create environment"
    echo "  pyp list                List environments"
    echo ""
    echo "For more information, see the README file."
    echo ""
}

# Main function
main() {
    UNINSTALL=0
    
    # Parse arguments
    parse_args "$@"
    
    # Check for uninstall
    if [[ $UNINSTALL -eq 1 ]]; then
        uninstall
        exit 0
    fi
    
    # Print banner
    echo ""
    echo "pyp C++ - Secure Python Virtual Environment Manager"
    echo "===================================================="
    echo ""
    
    # Check if running as root
    check_root
    
    # Check dependencies
    check_dependencies
    
    # Build the project
    build_project
    
    # Install binary
    install_binary
    
    # Install shell completion
    install_completion
    
    # Create wrapper
    create_wrapper
    
    # Create symlink
    create_symlink
    
    # Setup environment
    setup_environment
    
    # Run tests
    run_tests
    
    # Print summary
    print_summary
}

# Run main
main "$@"
