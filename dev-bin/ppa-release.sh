#!/bin/bash

set -e
set -x

DISTS=( focal eoan bionic xenial trusty )

VERSION=$(perl -MFile::Slurp::Tiny=read_file -MDateTime <<EOF
use v5.16;
my \$log = read_file(q{ChangeLog});
\$log =~ /(\d+\.\d+\.\d+) (\d{4}-\d{2}-\d{2})\n/;
die "Release time ($2) is not today!" unless DateTime->now->ymd eq \$2;
say \$1;
EOF
)

SRCDIST="GeoIP-$VERSION.tar.gz"
SRC=/tmp/GeoIP-$VERSION/
ORIG_NAME="geoip_$VERSION.orig.tar.gz"
RESULTS=/tmp/build-geoip-results/

tar xfvz "$SRCDIST" -C /tmp
cp -a debian "$SRC"

mkdir -p $RESULTS

for dist in "${DISTS[@]}"; do
    distdir=$(mktemp -d)
    cp -r "$SRC/" "$distdir/"
    cp "$SRCDIST" "$distdir/$ORIG_NAME"
    pushd "$distdir/GeoIP-$VERSION/"
    dch -v "$VERSION-0+maxmind1~$dist" -D "$dist" -u low "New upstream release."
    debuild -S -sa -rfakeroot
    popd
    ls "$distdir"
    mkdir -p "$RESULTS/$dist"
    cp "$distdir"/geoip_* "$RESULTS/$dist/"
    cp "$distdir/GeoIP-$VERSION/debian/changelog" "$RESULTS/$dist/changelog"
    rm -rf "$distdir"
done

read -e -p "Release to PPA? (y/n)" SHOULD_RELEASE

if [ "$SHOULD_RELEASE" != "y" ]; then
    echo "Aborting"
    exit 1
fi

dput ppa:maxmind/ppa "$RESULTS"/*/*source.changes


dch -v "$VERSION-0+maxmind1" -D "${DISTS[0]}" -u low "New upstream release."
git add debian/changelog
git commit -m "Update debian/changelog for $VERSION"
git push
