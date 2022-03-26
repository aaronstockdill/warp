warp () {
    if [[ $1 == "jump" || $1 == "remove" ]]; then
       if [[ $2 == "" ]]; then
           cd "$(warptool jump $(warptool list | fzf) --no-prompt || echo '.')"
       elif [[ $2 == "-h" || $2 == "--help" ]]; then
           warptool "$@"
       else
           cd "$(warptool "$@" || echo '.')"
       fi
    else
        warptool "$@"
    fi
    return $?
}
