## Releasing

* Make sure the `ChangeLog` is up to date.
* Run the following:

        ./bootstrap
        ./configure
        make check -j 4
        sudo make install

* Edit `configure.ac` and bump the version
* `make dist`
* Check that you can untar this release and install it
* `git tag v{X.Y.Z}`
* `git push --tags`
* Make a new release on GitHub at https://github.com/maxmind/geoip-api-c/releases
    * Upload the tarball you just made
    * Edit said release to include the changes for this release on GitHub
