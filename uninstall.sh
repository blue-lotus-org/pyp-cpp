#!/bin/bash
#
# pyp C++ - Uninstall Script
# Secure Python Virtual Environment Manager
#
# This script removes pyp (C++ version) from your system.
#
# Usage:
#   ./uninstall.sh [options]
#
# Options:
#   --prefix=PATH     Installation prefix (default: /usr/local)
#   --bindir=PATH     Binary directory (default: bin)
#   --local           Uninstall from $HOME instead of system directories
#   --keep-data       Keep user data and configurations
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
LOCALSTATEDIR="${LOCALSTATEDIR:-$PREFIX/var}"
DATADIR="${DATADIR:-$PREFIX/share}"

# Installation directories
INSTALL_BINDIR="$PREFIX/$BINDIR"
INSTALL_SYSCONFDIR="$PREFIX/$CONFDIR/pyp"
INSTALL_LOCALSTATEDIR="$LOCALSTATEDIR/lib/pyp"
INSTALL_DATADIR="$DATADIR/bash-completion/completions"
INSTALL_ZSHDIR="$DATADIR/zsh/site-functions"

# Binary names
BINARY_NAME="pyp"
BINARY_BACKUP="pyp.bin"

# User directories
USER_PYP_DIR="${HOME}/.pyp"
USER_ENVS_DIR="${USER_PYP_DIR}/envs"
USER_CONFIG_FILE="${USER_PYP_DIR}/config.json"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
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

print_header() {
    echo ""
    echo -e "${BOLD}${CYAN}=== $1 ===${NC}"
    echo ""
}

# Show help
show_help() {
    cat << EOF
pyp C++ - Uninstall Script

Usage: $0 [options]

Options:
  --prefix=PATH     Installation prefix (default: /usr/local)
  --bindir=PATH     Binary directory (default: bin)
  --local           Uninstall from \$HOME instead of system directories
  --keep-data       Keep user data and configurations
  --force           Don't ask for confirmation
  --help            Show this help message

Examples:
  $0                          # Standard uninstall
  $0 --local                  # Uninstall from \$HOME
  $0 --keep-data              # Keep user data
  $0 --force                  # Don't ask for confirmation

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
            --local)
                PREFIX="$HOME/.local"
                shift
                ;;
            --keep-data)
                KEEP_DATA=1
                shift
                ;;
            --force)
                FORCE=1
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

# Check permissions
check_permissions() {
    print_info "Checking permissions..."
    
    if [[ $EUID -eq 0 ]]; then
        SYSTEM_INSTALL=1
        print_info "Running as root - will remove system-wide installation"
    else
        SYSTEM_INSTALL=0
        print_info "Running as regular user - will remove user installation"
    fi
}

# Count environments to be removed
count_environments() {
    local env_count=0
    
    if [[ -d "$USER_ENVS_DIR" ]]; then
        env_count=$(find "$USER_ENVS_DIR" -maxdepth 1 -mindepth 1 -type d 2>/dev/null | wc -l)
    fi
    
    if [[ -d "$INSTALL_LOCALSTATEDIR/envs" ]]; then
        local sys_env_count=$(find "$INSTALL_LOCALSTATEDIR/envs" -maxdepth 1 -mindepth 1 -type d 2>/dev/null | wc -l)
        env_count=$((env_count + sys_env_count))
    fi
    
    echo $env_count
}

# Show what will be removed
show_removal_plan() {
    print_header "Removal Plan"
    
    echo "The following will be removed:"
    echo ""
    
    # Binary files
    echo "Binary files:"
    [[ -f "$INSTALL_BINDIR/$BINARY_NAME" ]] && echo "  - $INSTALL_BINDIR/$BINARY_NAME"
    [[ -f "$INSTALL_BINDIR/$BINARY_BACKUP" ]] && echo "  - $INSTALL_BINDIR/$BINARY_BACKUP"
    
    # Symlinks
    if [[ -L /usr/local/bin/pyp ]]; then
        echo "  - /usr/local/bin/pyp (symlink)"
    fi
    
    # Completion files
    echo ""
    echo "Shell completion:"
    [[ -f "$INSTALL_DATADIR/pyp" ]] && echo "  - $INSTALL_DATADIR/pyp"
    [[ -f "$INSTALL_ZSHDIR/_pyp" ]] && echo "  - $INSTALL_ZSHDIR/_pyp"
    
    # User data
    if [[ $KEEP_DATA -eq 0 ]]; then
        local env_count=$(count_environments)
        echo ""
        echo "User data (environments will be lost):"
        echo "  - $USER_PYP_DIR"
        echo "    ($env_count environment(s) will be removed)"
    else
        echo ""
        echo "User data (will be kept):"
        echo "  - $USER_PYP_DIR"
    fi
    
    echo ""
}

# Confirm removal
confirm_removal() {
    if [[ $FORCE -eq 1 ]]; then
        return 0
    fi
    
    echo ""
    print_warning "This will remove pyp and all its components from your system."
    echo ""
    
    read -p "Are you sure you want to continue? [y/N] " -n 1 -r
    echo ""
    
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_info "Uninstall cancelled"
        exit 0
    fi
}

# Remove binary files
remove_binary() {
    print_header "Removing Binary Files"
    
    local removed=0
    
    # Main binary
    if [[ -f "$INSTALL_BINDIR/$BINARY_NAME" ]]; then
        rm -f "$INSTALL_BINDIR/$BINARY_NAME"
        print_success "Removed: $INSTALL_BINDIR/$BINARY_NAME"
        removed=1
    else
        print_info "Not found: $INSTALL_BINDIR/$BINARY_NAME"
    fi
    
    # Backup binary
    if [[ -f "$INSTALL_BINDIR/$BINARY_BACKUP" ]]; then
        rm -f "$INSTALL_BINDIR/$BINARY_BACKUP"
        print_success "Removed: $INSTALL_BINDIR/$BINARY_BACKUP"
        removed=1
    fi
    
    # Symlinks
    if [[ -L /usr/local/bin/pyp ]]; then
        rm -f /usr/local/bin/pyp
        print_success "Removed symlink: /usr/local/bin/pyp"
        removed=1
    fi
    
    # Try ~/bin symlink
    if [[ -L "$HOME/bin/pyp" ]]; then
        rm -f "$HOME/bin/pyp"
        print_success "Removed symlink: $HOME/bin/pyp"
    fi
    
    # Remove empty directories
    if [[ $removed -eq 1 ]]; then
        rmdir "$INSTALL_BINDIR" 2>/dev/null || true
        rmdir "$INSTALL_SYSCONFDIR" 2>/dev/null || true
        rmdir "$INSTALL_LOCALSTATEDIR" 2>/dev/null || true
        print_info "Cleaned up empty directories"
    fi
}

# Remove shell completion
remove_completion() {
    print_header "Removing Shell Completion"
    
    # Bash completion
    if [[ -f "$INSTALL_DATADIR/pyp" ]]; then
        rm -f "$INSTALL_DATADIR/pyp"
        print_success "Removed bash completion: $INSTALL_DATADIR/pyp"
    else
        print_info "Bash completion not found"
    fi
    
    # Zsh completion
    if [[ -f "$INSTALL_ZSHDIR/_pyp" ]]; then
        rm -f "$INSTALL_ZSHDIR/_pyp"
        print_success "Removed zsh completion: $INSTALL_ZSHDIR/_pyp"
    else
        print_info "Zsh completion not found"
    fi
    
    # Remove empty directories
    rmdir "$INSTALL_DATADIR" 2>/dev/null || true
    rmdir "$INSTALL_ZSHDIR" 2>/dev/null || true
}

# Remove user data
remove_user_data() {
    if [[ $KEEP_DATA -eq 1 ]]; then
        print_header "Keeping User Data"
        print_info "User data preserved at: $USER_PYP_DIR"
        return 0
    fi
    
    print_header "Removing User Data"
    
    if [[ -d "$USER_PYP_DIR" ]]; then
        local env_count=$(count_environments)
        
        rm -rf "$USER_PYP_DIR"
        print_success "Removed user data: $USER_PYP_DIR"
        
        if [[ $env_count -gt 0 ]]; then
            print_info "Removed $env_count environment(s)"
        fi
    else
        print_info "No user data found at: $USER_PYP_DIR"
    fi
}

# Remove environment variables from shell config
cleanup_shell_config() {
    print_header "Cleaning Shell Configuration"
    
    print_info "You may want to remove pyp-related lines from your shell config files:"
    echo ""
    echo "  ~/.bashrc"
    echo "  ~/.bash_profile"
    echo "  ~/.zshrc"
    echo ""
    print_info "Look for lines containing: pyp, PYP_HOME, PYP_CURRENT_ENV"
    echo ""
}

# Remove from package manager (if installed via package)
remove_package_manager() {
    # Check for dpkg/rpm packages
    if command -v dpkg &> /dev/null; then
        if dpkg -l pyp &> /dev/null 2>&1; then
            print_warning "pyp appears to be installed via package manager"
            print_info "To fully remove, run: sudo apt-get remove pyp"
        fi
    fi
    
    if command -v rpm &> /dev/null; then
        if rpm -q pyp &> /dev/null 2>&1; then
            print_warning "pyp appears to be installed via package manager"
            print_info "To fully remove, run: sudo rpm -e pyp"
        fi
    fi
}

# Print uninstallation summary
print_summary() {
    echo ""
    echo "========================================"
    print_success "Uninstallation Complete!"
    echo "========================================"
    echo ""
    echo "pyp has been removed from your system."
    echo ""
    
    if [[ $KEEP_DATA -eq 0 ]]; then
        echo "All environments and configurations have been deleted."
    else
        echo "Your environments and configurations have been preserved."
    fi
    echo ""
    
    print_info "Thank you for using pyp!"
    echo ""
}

# Main function
main() {
    KEEP_DATA=0
    FORCE=0
    
    # Parse arguments
    parse_args "$@"
    
    # Print banner
    echo ""
    echo "pyp C++ - Uninstall Script"
    echo "=========================="
    echo ""
    
    # Check permissions
    check_permissions
    
    # Show removal plan
    show_removal_plan
    
    # Confirm removal
    confirm_removal
    
    # Remove binary files
    remove_binary
    
    # Remove shell completion
    remove_completion
    
    # Remove user data
    remove_user_data
    
    # Clean up shell config
    cleanup_shell_config
    
    # Check for package manager installation
    remove_package_manager
    
    # Print summary
    print_summary
}

# Run main
main "$@"
