if command -v fzf &> /dev/null ; then
    warp () {
        if [[ $1 == "jump" || $1 == "remove" ]]; then
            if [[ $2 == "" ]]; then
                cd "$(warptool jump $(warptool list | fzf) --no-prompt || echo '.')"
            elif [[ $2 == "-h" || $2 == "--help" ]]; then
                warptool "$@"
            else
                cd "$(warptool $@ || echo '.')"
            fi
        else
            warptool "$@"
        fi
        local result=$?
        if [[ $result == 2 ]]; then
            return 0
        else
            return $result
        fi
    }
else
    warp () {
        if [[ $1 == "jump" || $1 == "remove" ]]; then
            if [[ $2 == "" ]]; then
                echo -n 'Warp point name: '
                read name
                cd "$(warptool jump $name --no-prompt || echo '.')"
            elif [[ $2 == "-h" || $2 == "--help" ]]; then
                warptool "$@"
            else
                cd "$(warptool $@ || echo '.')"
            fi
        else
            warptool "$@"
        fi
        local result=$?
        if [[ $result == 2 ]]; then
            return 0
        else
            return $result
        fi
    }
fi
