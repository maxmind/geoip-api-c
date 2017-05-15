#!/bin/bash

set -eu -o pipefail

changelog=$(cat ChangeLog)

regex='([0-9]+\.[0-9]+\.[0-9]+) ([0-9]{4}-[0-9]{2}-[0-9]{2})

((.|
)*)
'

if [[ ! $changelog =~ $regex ]]; then
      echo "Could not find date line in change log!"
      exit 1
fi

version="${BASH_REMATCH[1]}"
date="${BASH_REMATCH[2]}"
notes="$(echo "${BASH_REMATCH[3]}" | sed -n -e '/^[0-9]\+\.[0-9]\+\.[0-9]\+/,$!p')"

dist="GeoIP-$version.tar.gz"

if [[ "$date" !=  $(date +"%Y-%m-%d") ]]; then
    echo "$date is not today!"
    exit 1
fi

if [ -n "$(git status --porcelain)" ]; then
    echo ". is not clean." >&2
    exit 1
fi

perl -pi -e "s/(?<=AC_INIT\(\[GeoIP\], \[)(\d+\.\d+\.\d+)(?=])/$version/gsm" configure.ac

if [ -n "$(git status --porcelain)" ]; then
    git add configure.ac 
    git commit -m "Bumped version to $version"
fi

./bootstrap
./configure
make
make check
make clean
make dist

read -p "Push to origin? (y/n) " should_push

if [ "$should_push" != "y" ]; then
    echo "Aborting"
    exit 1
fi

git push

message="$version

$notes"

hub release create -a "$dist" -m "$message" "v$version"
