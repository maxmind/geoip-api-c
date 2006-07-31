NULL =

INCLUDES =	\
	-I$(top_srcdir)/libGeoIP		\
	-Wall -ansi				\
	$(NULL)

check_PROGRAMS = test-geoip

#if WITH_CITY
noinst_PROGRAMS = benchmark test-geoip-region test-geoip-city test-geoip-org test-geoip-asnum test-geoip-isp test-geoip-netspeed
#else
#noinst_PROGRAMS = benchmark test-geoip-region
#endif

DEPS = $(top_builddir)/libGeoIP/libGeoIP.la
LDADDS = $(top_builddir)/libGeoIP/libGeoIP.la

test_geoip_SOURCES = test-geoip.c
test_geoip_LDFLAGS =
test_geoip_DEPENDENCIES = $(DEPS)
test_geoip_LDADD = $(LDADDS)

test_geoip_region_SOURCES = test-geoip-region.c
test_geoip_region_LDFLAGS =
test_geoip_region_DEPENDENCIES = $(DEPS)
test_geoip_region_LDADD = $(LDADDS)

test_geoip_org_SOURCES = test-geoip-org.c
test_geoip_org_LDFLAGS =
test_geoip_org_DEPENDENCIES = $(DEPS)
test_geoip_org_LDADD = $(LDADDS)

test_geoip_isp_SOURCES = test-geoip-isp.c
test_geoip_isp_LDFLAGS =
test_geoip_isp_DEPENDENCIES = $(DEPS)
test_geoip_isp_LDADD = $(LDADDS)

test_geoip_asnum_SOURCES = test-geoip-asnum.c
test_geoip_asnum_LDFLAGS =
test_geoip_asnum_DEPENDENCIES = $(DEPS)
test_geoip_asnum_LDADD = $(LDADDS)

test_geoip_netspeed_SOURCES = test-geoip-netspeed.c
test_geoip_netspeed_LDFLAGS =
test_geoip_netspeed_DEPENDENCIES = $(DEPS)
test_geoip_netspeed_LDADD = $(LDADDS)

if WITH_CITY
test_geoip_city_SOURCES = test-geoip-city.c
test_geoip_city_LDFLAGS =
test_geoip_city_DEPENDENCIES = $(DEPS)
test_geoip_city_LDADD = $(LDADDS)
endif

benchmark_SOURCES = benchmark.c
benchmark_LDFLAGS =
benchmark_DEPENDENCIES = $(DEPS)
benchmark_LDADD = $(LDADDS)

EXTRA_DIST = country_test.txt country_test2.txt country_test_name.txt region_test.txt
TESTS = test-geoip
