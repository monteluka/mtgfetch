function _mtgfetch_completion()
{
  local LATEST_WORD="${COMP_WORDS[$COMP_CWORD]}"
  local WORD_COUNT="${#COMP_WORDS[@]}"

  local MTGF_OPTIONS=("-h" "--help" "-v" "--version" "-r" "--random" "--gen-config" "--gen-config-all")

  if [[ $WORD_COUNT -lt 3 ]]; then
    COMPREPLY=($(compgen -W "${MTGF_OPTIONS[*]}" -- "$LATEST_WORD"))
  fi

  return 0
}

complete -F _mtgfetch_completion mtgfetch