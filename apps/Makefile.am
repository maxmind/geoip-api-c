INCLUDES = 	\
	-I$(top_srcdir)/libGeoIP		\
	-Wall

AM_CPPFLAGS = -DSYSCONFDIR=\"$(sysconfdir)\" -Wall

DEPS = $(top_builddir)/libGeoIP/libGeoIP.la
LDADDS = $(top_builddir)/libGeoIP/libGeoIP.la

bin_PROGRAMS = geoiplookup geoipupdate geoipexport

geoiplookup_SOURCES = geoiplookup.c
geoiplookup_LDFLAGS =
geoiplookup_DEPENDENCIES = $(DEPS)
geoiplookup_LDADD = $(LDADDS)

geoipupdate_SOURCES = geoipupdate.c
geoipupdate_LDFLAGS =
geoipupdate_DEPENDENCIES = $(DEPS)
geoipupdate_LDADD = $(LDADDS)

geoipexport_SOURCES = geoipexport.c
geoipexport_LDFLAGS =
geoipexport_DEPENDENCIES = $(DEPS)
geoipexport_LDADD = $(LDADDS)
