## Releasing

* Make sure https://github.com/github/hub is installed and configured.
* Make sure the `ChangeLog` is up to date.
* Run `dev-bin/release.sh`

## Ubuntu PPA packages

0. Switch to the `ubuntu-ppa` branch and merge the release tag from above.

Release script:

1. run `dev-bin/ppa-release.sh`

Manual PPA process:

1. Type `dch -i` and add the appropriate `debian/changelog` entry.
2. Move tarball created above to a temp directory and
   name it `geoip_1.?.?.orig.tar.gz`.
3. Unpack tarball.
4. Copy `debian` directory from Git. (We intentionally do not include it in
   the tarball so that we don't interfere with Debian's packaging.)
5. Update `debian/changelog` for the dist you are releasing to, e.g.,
   precise, trusty, vivid, and prefix the version with the a `~` followed
   by the dist name, e.g., `1.6.3-1+maxmind1~trusty`.
6. Run `debuild -S -sa -rfakeroot -k<KEY>`. (The key may not be necessary
   if your .bashrc is appropriately )
7. Run `lintian` to make sure everything looks sane.
8. Run `dput ppa:maxmind/ppa ../<source.changes files created above>` to
   upload.
9. Repeat 4-8 for remaining distributions.

## Homebrew

* Update the [Homebrew formula](https://github.com/Homebrew/homebrew-core/blob/master/Formula/geoip.rb).
