man_MANS = geoiplookup.1

EXTRA_DIST = geoiplookup.1 geoipupdate.1

DEFAULT_MAN = $(mandir)/man1/geoipupdate.1

install-data-hook: install-man
	cat geoipupdate.1 | sed s,CONF_DIR,$(sysconfdir), > $(DESTDIR)$(DEFAULT_MAN)
