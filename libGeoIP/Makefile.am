lib_LTLIBRARIES = libGeoIP.la

EXTRA_DIST = md5.h global.h

AM_CPPFLAGS = -DGEOIPDATADIR=\"$(pkgdatadir)\" -Wall

if WITH_CITY
libGeoIP_la_SOURCES = GeoIP.c GeoIPUpdate.c md5.c GeoIPCity.c
include_HEADERS = GeoIP.h GeoIPUpdate.h GeoIPCity.h
else
libGeoIP_la_SOURCES = GeoIP.c GeoIPUpdate.c md5.c
include_HEADERS = GeoIP.h GeoIPUpdate.h
endif

libGeoIP_la_LIBADD = -lz
libGeoIP_la_LDFLAGS = -version-info @GEOIP_VERSION_INFO@
