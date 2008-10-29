man_MANS = geoiplookup.1

EXTRA_DIST = geoiplookup6.1 geoiplookup.1 geoipupdate.1

UPDATE_MAN = $(mandir)/man1/geoipupdate.1
LOOKUP_MAN = $(mandir)/man1/geoiplookup.1
LOOKUP6_MAN = $(mandir)/man1/geoiplookup6.1

install-data-hook: install-man
	cat geoipupdate.1 | sed s,DATADIR,$(pkgdatadir), | sed s,CONF_DIR,$(sysconfdir), > $(DESTDIR)$(UPDATE_MAN)
	cat geoiplookup.1 | sed s,DATADIR,$(pkgdatadir), > $(DESTDIR)$(LOOKUP_MAN)
	cat geoiplookup6.1 | sed s,DATADIR,$(pkgdatadir), > $(DESTDIR)$(LOOKUP6_MAN)

