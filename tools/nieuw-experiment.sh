#!/usr/bin/env bash
# Make a new experiment from experimenten/_sjabloon.
#
# Usage: tools/nieuw-experiment.sh <id> <short-name>
#   id          e002, e003, ...  (lower case, three digits)
#   short-name  words joined by '-', for example iterator-invalidation
#
# The script copies the template, fills in the id, and adds the directory to
# experimenten/CMakeLists.txt. It does not touch the catalogue or the log.
set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "usage: $0 <id> <short-name>   (example: $0 e002 iterator-invalidation)" >&2
    exit 2
fi

id="$1"
name="$2"
if [[ ! "$id" =~ ^e[0-9]{3}$ ]]; then
    echo "error: id must look like e002, got '$id'" >&2
    exit 2
fi
if [[ ! "$name" =~ ^[a-z0-9]+(-[a-z0-9]+)*$ ]]; then
    echo "error: short-name must be lower case words joined by '-', got '$name'" >&2
    exit 2
fi

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
dir="experimenten/${id}-${name}"
if compgen -G "$root/experimenten/${id}-*" > /dev/null; then
    echo "error: an experiment with id $id exists already" >&2
    exit 1
fi

cp -r "$root/experimenten/_sjabloon" "$root/$dir"
title="${name//-/ }"
today="$(date +%Y-%m-%d)"
find "$root/$dir" -type f -exec sed -i \
    -e "s/@ID@/${id}/g" -e "s/@TITEL@/${title}/g" -e "s/@DATUM@/${today}/g" {} +

echo "add_subdirectory(${id}-${name})" >> "$root/experimenten/CMakeLists.txt"

echo "Made $dir"
echo "Next: set the catalogue entry to 'bezig', write the question and expectation in $dir/README.md"
