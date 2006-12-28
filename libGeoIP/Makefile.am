lib_LTLIBRARIES = libGeoIP.la libGeoIPUpdate.la

EXTRA_DIST = md5.h global.h types.h GeoIP_internal.h

AM_CPPFLAGS = -DGEOIPDATADIR=\"$(pkgdatadir)\" -Wall

if WITH_CITY
libGeoIP_la_SOURCES = GeoIP.c GeoIPCity.c regionName.c timeZone.c
include_HEADERS = GeoIP.h GeoIPCity.h GeoIPUpdate.h
else
libGeoIP_la_SOURCES = GeoIP.c regionName.c timeZone.c
include_HEADERS = GeoIP.h GeoIPUpdate.h
endif

libGeoIPUpdate_la_SOURCES = GeoIPUpdate.c md5.c

libGeoIP_la_LDFLAGS = -version-info @GEOIP_VERSION_INFO@

libGeoIPUpdate_la_LIBADD = -lz libGeoIP.la
