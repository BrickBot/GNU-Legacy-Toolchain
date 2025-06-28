#!/bin/bash
# Update config.guess and config.sub from upstream

SCRIPT_DIR=$(realpath -s $(dirname $0))
UPSTREAM_REPO=https://git.savannah.gnu.org/cgit/config.git/plain

FILES=( "config.guess" "config.sub" )

for FILE in "${FILES[@]}"; do
  wget --output-document="$SCRIPT_DIR/$FILE" "$UPSTREAM_REPO/$FILE"
done
