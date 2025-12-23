# pyp bash completion
# Secure Python Virtual Environment Manager
#
# Install by sourcing this file or copying to:
# - /etc/bash_completion.d/pyp
# - ~/.bash_completion.d/pyp
# - ~/.bash_completion.d/

_pyp() {
    local cur prev words cword
    _init_completion || return

    # Main commands
    local commands="build activate deactivate list remove info run export import set-default cleanup init help"

    # Command aliases
    local aliases_build="create new make"
    local aliases_activate="use enter source"
    local aliases_deactivate="exit leave"
    local aliases_list="ls show dir"
    local aliases_remove="rm delete del"
    local aliases_info="show details status"
    local aliases_run="exec execute"
    local aliases_export="save dump"
    local aliases_import="load restore"
    local aliases_set_default="default"
    local aliases_cleanup="clean gc"
    local aliases_init="initialize setup"

    # Options
    local options="--help --version --python --upgrade --force --output --name --active --default"

    # Python versions
    local python_versions="python3 python python2 python3.11 python3.10 python3.9 python3.8"

    # First argument - command or option
    if [[ $cword -eq 1 ]]; then
        COMPREPLY=($(compgen -W "$commands $options" -- "$cur"))
        return
    fi

    # Get the command (handle aliases)
    local cmd="${words[1]}"
    case "$cmd" in
        build|create|new|make)
            if [[ $cword -eq 2 ]]; then
                # Environment name - suggest a valid name
                COMPREPLY=($(compgen -W "myenv dev prod test staging $(ls ~/.pyp/envs 2>/dev/null)" -- "$cur"))
                _compopt_o_filenames
            else
                # Options for build command
                COMPREPLY=($(compgen -W "--python --upgrade --help" -P "--" -- "$cur"))
            fi
            ;;
        activate|use|enter|source)
            if [[ $cword -eq 2 ]]; then
                # List environments
                COMPREPLY=($(compgen -W "$(ls ~/.pyp/envs 2>/dev/null)" -- "$cur"))
                _compopt_o_filenames
            fi
            ;;
        deactivate|exit|leave)
            # No arguments needed
            ;;
        list|ls|show|dir)
            if [[ $cword -eq 2 ]]; then
                COMPREPLY=($(compgen -W "--active --default --help" -P "--" -- "$cur"))
            fi
            ;;
        remove|rm|delete|del)
            if [[ $cword -eq 2 ]]; then
                COMPREPLY=($(compgen -W "$(ls ~/.pyp/envs 2>/dev/null)" -- "$cur"))
                _compopt_o_filenames
            else
                COMPREPLY=($(compgen -W "--force --help" -P "--" -- "$cur"))
            fi
            ;;
        info|show|details|status)
            if [[ $cword -eq 2 ]]; then
                COMPREPLY=($(compgen -W "$(ls ~/.pyp/envs 2>/dev/null)" -- "$cur"))
                _compopt_o_filenames
            fi
            ;;
        run|exec|execute)
            if [[ $cword -eq 2 ]]; then
                COMPREPLY=($(compgen -W "$(ls ~/.pyp/envs 2>/dev/null)" -- "$cur"))
                _compopt_o_filenames
            else
                # Command to run
                COMPREPLY=($(compgen -c -- "$cur"))
            fi
            ;;
        export|save|dump)
            if [[ $cword -eq 2 ]]; then
                COMPREPLY=($(compgen -W "$(ls ~/.pyp/envs 2>/dev/null)" -- "$cur"))
                _compopt_o_filenames
            else
                COMPREPLY=($(compgen -W "--output --help" -P "--" -- "$cur"))
                compopt -o default
            fi
            ;;
        import|load|restore)
            if [[ $cword -eq 2 ]]; then
                _filedir json
            else
                COMPREPLY=($(compgen -W "--name --help" -P "--" -- "$cur"))
            fi
            ;;
        set-default|default)
            if [[ $cword -eq 2 ]]; then
                COMPREPLY=($(compgen -W "$(ls ~/.pyp/envs 2>/dev/null)" -- "$cur"))
                _compopt_o_filenames
            fi
            ;;
        cleanup|clean|gc)
            # No arguments needed
            ;;
        init|initialize|setup)
            # No arguments needed
            ;;
        help)
            if [[ $cword -eq 2 ]]; then
                COMPREPLY=($(compgen -W "$commands" -- "$cur"))
            fi
            ;;
        --help|-h)
            return
            ;;
        --version|-v)
            return
            ;;
        *)
            # Unknown command
            ;;
    esac
} && complete -F _pyp pyp

# Alias completions
complete -F _pyp pyp-build 2>/dev/null || true
complete -F _pyp pyp-create 2>/dev/null || true
complete -F _pyp pyp-activate 2>/dev/null || true
complete -F _pyp pyp-use 2>/dev/null || true
complete -F _pyp pyp-list 2>/dev/null || true
complete -F _pyp pyp-ls 2>/dev/null || true
complete -F _pyp pyp-remove 2>/dev/null || true
complete -F _pyp pyp-rm 2>/dev/null || true

# Environment name completion helper
_pyp_envs() {
    local envs
    envs=$(ls ~/.pyp/envs 2>/dev/null)
    COMPREPLY=($(compgen -W "$envs" -- "${cur:-}"))
}

# Alias completion helpers
complete -F _pyp_envs pyp-activate 2>/dev/null || true
complete -F _pyp_envs pyp-use 2>/dev/null || true
complete -F _pyp_envs pyp-info 2>/dev/null || true
complete -F _pyp_envs pyp-remove 2>/dev/null || true
complete -F _pyp_envs pyp-rm 2>/dev/null || true
complete -F _pyp_envs pyp-run 2>/dev/null || true
complete -F _pyp_envs pyp-export 2>/dev/null || true
