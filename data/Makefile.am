pkgdata_DATA =

EXTRA_DIST = GeoIP.dat

DEFAULT_DB_FILE = $(pkgdatadir)/GeoIP.dat

install-data-hook: install-pkgdataDATA
	@if test -f $(DESTDIR)$(DEFAULT_DB_FILE) ; then \
	        echo "$@ will not overwrite existing $(DESTDIR)$(DEFAULT_DB_FILE)" ; \
	else \
	        echo "$(INSTALL_DATA) GeoIP.dat $(DESTDIR)$(DEFAULT_DB_FILE)"; \
	        $(INSTALL_DATA) GeoIP.dat $(DESTDIR)$(DEFAULT_DB_FILE); \
	fi
