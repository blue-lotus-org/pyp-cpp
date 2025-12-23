#compdef pyp
#
# pyp zsh completion
# Secure Python Virtual Environment Manager
#
# Install by copying this file to:
# - ~/.zsh/completion/_pyp
# - ~/.zsh/site-functions/_pyp
#

local curcontext="$curcontext" state line
typeset -A opt_args

_pyp_commands() {
    local commands
    commands=(
        'build:Create a new virtual environment'
        'activate:Activate a virtual environment'
        'deactivate:Deactivate the current environment'
        'list:List all virtual environments'
        'remove:Remove a virtual environment'
        'info:Show information about an environment'
        'run:Run a command in an environment'
        'export:Export environment to a file'
        'import:Import environment from a file'
        'set-default:Set the default environment'
        'cleanup:Clean up orphaned environments'
        'init:Initialize pyp configuration'
        'help:Show help information'
    )
    _describe -t commands 'pyp command' commands
}

_pyp_aliases() {
    local aliases
    aliases=(
        'build:create new make'
        'activate:use enter source'
        'deactivate:exit leave'
        'list:ls show dir'
        'remove:rm delete del'
        'info:show details status'
        'run:exec execute'
        'export:save dump'
        'import:load restore'
        'set-default:default'
        'cleanup:clean gc'
        'init:initialize setup'
    )
    _describe -t aliases 'pyp command aliases' aliases
}

_pyp_environments() {
    local envs
    envs=($(ls ~/.pyp/envs 2>/dev/null))
    _describe -t environments 'environments' envs
}

_pyp_options() {
    local options
    options=(
        '--help:Show help message'
        '--version:Show version information'
        '--python:Specify Python version'
        '--upgrade:Upgrade existing environment'
        '--force:Force operation without confirmation'
        '--output:Specify output file for export'
        '--name:Specify name for import'
        '--active:Show only active environment'
        '--default:Show only default environment'
    )
    _describe -t options 'options' options
}

_pyp_python_versions() {
    local versions
    versions=(
        'python3:Python 3.x'
        'python:Python (system default)'
        'python2:Python 2.x'
        'python3.11:Python 3.11'
        'python3.10:Python 3.10'
        'python3.9:Python 3.9'
        'python3.8:Python 3.8'
    )
    _describe -t python-versions 'Python versions' versions
}

_arguments -C \
    '1: :_pyp_commands' \
    '*::arg:->args'

case $state in
    args)
        case $line[1] in
            build|create|new|make)
                _arguments \
                    '1:environment name:_pyp_environments' \
                    '--python: :_pyp_python_versions' \
                    '--upgrade:boolean' \
                    '--help::boolean'
                ;;
            activate|use|enter|source)
                _arguments \
                    '1: :_pyp_environments' \
                    '--help::boolean'
                ;;
            deactivate|exit|leave)
                _arguments \
                    '--help::boolean'
                ;;
            list|ls|show|dir)
                _arguments \
                    '--active:boolean' \
                    '--default:boolean' \
                    '--help::boolean'
                ;;
            remove|rm|delete|del)
                _arguments \
                    '1: :_pyp_environments' \
                    '--force:boolean' \
                    '--help::boolean'
                ;;
            info|show|details|status)
                _arguments \
                    '1: :_pyp_environments' \
                    '--help::boolean'
                ;;
            run|exec|execute)
                _arguments \
                    '1: :_pyp_environments' \
                    '2:command: _command_names -e' \
                    '*:: :->run_args'
                case $state in
                    run_args)
                        _normal
                        ;;
                esac
                ;;
            export|save|dump)
                _arguments \
                    '1: :_pyp_environments' \
                    '--output: :_files -g "*.json"' \
                    '--help::boolean'
                ;;
            import|load|restore)
                _arguments \
                    '1: :_files -g "*.json"' \
                    '--name: :_pyp_env_name' \
                    '--help::boolean'
                ;;
            set-default|default)
                _arguments \
                    '1: :_pyp_environments' \
                    '--help::boolean'
                ;;
            cleanup|clean|gc)
                _arguments \
                    '--help::boolean'
                ;;
            init|initialize|setup)
                _arguments \
                    '--help::boolean'
                ;;
            help)
                _arguments \
                    '1: :_pyp_commands' \
                    '--help::boolean'
                ;;
        esac
        ;;
esac

# Helper function for environment name completion
_pyp_env_name() {
    local envs
    envs=($(ls ~/.pyp/envs 2>/dev/null))
    _describe -t env-names 'environment names' envs
}

# Alias command completions
(( $+functions[_pyp-build] )) || _pyp-build() {
    _arguments \
        '1:environment name:_pyp_environments' \
        '--python: :_pyp_python_versions' \
        '--upgrade:boolean' \
        '--help::boolean'
}

(( $+functions[_pyp-activate] )) || _pyp-activate() {
    _arguments \
        '1: :_pyp_environments' \
        '--help::boolean'
}

(( $+functions[_pyp-list] )) || _pyp-list() {
    _arguments \
        '--active:boolean' \
        '--default:boolean' \
        '--help::boolean'
}

(( $+functions[_pyp-remove] )) || _pyp-remove() {
    _arguments \
        '1: :_pyp_environments' \
        '--force:boolean' \
        '--help::boolean'
}

(( $+functions[_pyp-run] )) || _pyp-run() {
    _arguments \
        '1: :_pyp_environments' \
        '2:command: _command_names -e' \
        '*:: :->run_args'
    case $state in
        run_args)
            _normal
            ;;
    esac
}

# Complete environment names for all commands that need them
_pyp_all_envs() {
    _arguments '*:environment:_pyp_environments'
}
