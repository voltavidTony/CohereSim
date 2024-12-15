#!/bin/bash

# Usage message when no arguments supplied
if [ $# -eq 0 ]; then
  echo "Usage:"
  echo "  ./mksrc.sh coherence <class_name> <states..>"
  echo "  ./mksrc.sh replacement|textbook <class_name>"
  echo "  ./mksrc.sh <file_name>"
  echo "Options:"
  echo "  class_name: The name of the new policy/protocol (TitleCase)"
  echo "  file_name:  The file name of the generic source file pair (snake_case)"
  echo "  states:     The list of states in the coherence protocol's state transition graph."
  echo "                Choose at least two of I, D, E, M, V, O, S, Sc, and Sm"
  exit 0
fi

# Obtain directories
cd "$(dirname "$0")/.."
SRCDIR="$(pwd)/src"
TMPLTDIR="$(pwd)/template"

# Create a blank source and header file pair
if [ $# -eq 1 ]; then
    # File name must be valid snake case
    if [[ ! $1 =~ ^[a-z]+(_[a-z]+)*$ ]]; then
      echo "The filename must be in snake case using only english letters!"
      exit 1
    fi
    # Write generic files
    echo -e "/// @file $1.h\n/// @brief\n\n#pragma once" > "$SRCDIR/$1.h"
    echo -e "/// @file $1.cc\n/// @brief\n\n#include \"$1.h\"" > "$SRCDIR/$1.cc"
    exit 0
fi

# Create a templated source and header file pair
if [ $# -gt 1 ]; then
  # Policy/Protocol must be valid title case
  if [[ ! $2 =~ ^([A-Z][a-z]*)+$ ]]; then
    echo "The policy/protocol name must be in title case using only english letters!"
    exit 2
  fi
  # Convert title case to snake case
  fname=$(echo $2 | perl -pe 's/([A-Z])(?=[A-Z][a-z])/\1_/g;s/([a-z])(?=[A-Z])/\1_/g')
  fname=${fname,,}

  # Template option specific values
  states=""
  case $1 in
    "replacement")
      ;;

    "textbook")
      ;;

    "coherence")
      # Rule 1: At least 2 states
      if [ $# -le 3 ]; then
        echo "Please supply at least 2 states!"
        exit 3
      fi
      for state in "${@:3}"; do
        # Rule 2: No duplicate state
        if [[ $states =~ $state ]]; then
          echo "Duplicate state: $state"
          exit 3
        fi
        # Rule 3: Only specific states permissible
        if ! [[ "$state" =~ ^(D|E|I|M|O|S|(Sc)|(Sm)|V)$ ]]; then
          echo "Illegal state: $state"
          exit 3
        fi
        states="$states    case $state:\n"
      done
      states="$states    default:"
      ;;

    *)
      echo "Unknown template option: $1. Only 'coherence', 'replacement', and 'textbook' are available!"
      exit 1
      ;;
  esac

  # Write template files
  sed -e "s/__FILE__/$fname/g;s/__CLASS__/$2/g;s/__STATES__/$states/g" "$TMPLTDIR/$1_h.tmplt" > "$SRCDIR/$1/$fname.h"
  sed -e "s/__FILE__/$fname/g;s/__CLASS__/$2/g;s/__STATES__/$states/g" "$TMPLTDIR/$1_c.tmplt" > "$SRCDIR/$1/$fname.cc"
fi
