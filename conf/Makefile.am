sysconf_DATA = GeoIP.conf.default

EXTRA_DIST = GeoIP.conf.default

DEFAULT_CONFIG_FILE = $(sysconfdir)/GeoIP.conf

install-data-hook: install-sysconfDATA
	@if test -f $(DESTDIR)$(DEFAULT_CONFIG_FILE) ; then \
	        echo "$@ will not overwrite existing $(DESTDIR)$(DEFAULT_CONFIG_FILE)" ; \
	else \
	        echo "$(INSTALL_DATA) GeoIP.conf.default $(DESTDIR)$(DEFAULT_CONFIG_FILE)"; \
	        $(INSTALL_DATA) GeoIP.conf.default $(DESTDIR)$(DEFAULT_CONFIG_FILE); \
	fi
