#ifndef GEOIP_INTERNAL_H
#define GEOIP_INTERNAL_H

GEOIP_API unsigned int _GeoIP_seek_record (GeoIP *gi, unsigned long ipnum);
GEOIP_API unsigned long _GeoIP_addr_to_num (const char *addr);

unsigned long _GeoIP_lookupaddress (const char *host);
extern void _GeoIP_setup_dbfilename();
extern char *_GeoIP_full_path_to(const char *file_name);

#endif
