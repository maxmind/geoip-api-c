/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/* geoipupdate.c
 *
 * Copyright (C) 2002 MaxMind.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define _GNU_SOURCE
#include "GeoIPBitReader.h"
#include "GeoIP.h"
#include <math.h>

const int EXPORT_BINARY   = 1;
const int EXPORT_CSV      = 2;
const int EXPORT_FULL_CSV = 3;
const int EXPORT_XML      = 4;

const int COUNTRY_RECORD_SIZE = 8;
const int CITY_RECORD_SIZE = 20;

void usage() {
  fprintf(stderr,"Usage: geoipexport [-bcfx] bit_file out_file1 [out_file2]\n");
}

char * _num_to_addr (unsigned long num) {
	char * addr = malloc(15);
	sprintf(addr, "%u.%u.%u.%u",
					 (int)floor(num/16777216),
					 ((int)floor(num/65536)) % 256,
					 ((int)floor(num/256)) % 256,
					 (int)(num % 256));
	return addr;
}

void full_csv_export (int databaseType, int beginIp, int endIp, int val, FILE *out) {
	if (GEOIP_COUNTRY_EDITION == databaseType) {
		fprintf(out, "\"%s\",\"%s\",\"%u\",\"%u\",\"%s\",\"%s\"\n",
						_num_to_addr(beginIp), _num_to_addr(endIp), beginIp, endIp,
						GeoIP_country_code[val], GeoIP_country_name[val]);
	}
}

void csv_export (int databaseType, int beginIp, int endIp, int val, FILE *out) {
	if (GEOIP_COUNTRY_EDITION == databaseType) {
		fprintf(out, "\"%u\",\"%u\",\"%s\"\n",
						beginIp, endIp, GeoIP_country_code[val]);
	} else if (GEOIP_CITY_EDITION_REV0 == databaseType) {
		fprintf(out, "\"%u\",\"%u\",\"%u\"\n",
						beginIp, endIp, val);
	}
}

void csv_export2 (int databaseType, int beginIp, int endIp, int val, FILE *out) {
	if (GEOIP_COUNTRY_EDITION == databaseType) {
		fprintf(out, "\"%s\",\"%s\",\"%s\"\n",
						_num_to_addr(beginIp), _num_to_addr(endIp), GeoIP_country_code[val]);
	} else if (GEOIP_CITY_EDITION_REV0 == databaseType) {
		fprintf(out, "\"%s\",\"%s\",\"%u\"\n",
						_num_to_addr(beginIp), _num_to_addr(endIp), val);
	}
}

int main (int argc, char *argv[]) {
  FILE *f;
  GeoIPBitReader *gibr;
	int databaseType, record, val = 0;
	int exportType = 0;
	unsigned long beginIp = 0, endIp = 0;

  if (argc < 4) {
    usage();
    exit(1);
  }

	if (strcmp(argv[1],"-b") == 0) {
		exportType = EXPORT_BINARY;
	} else if (strcmp(argv[1],"-c") == 0) {
		exportType = EXPORT_CSV;
	} else if (strcmp(argv[1],"-f") == 0) {
		exportType = EXPORT_FULL_CSV;
	} else if (strcmp(argv[1],"-x") == 0) {
		exportType = EXPORT_XML;
	}

  f = fopen(argv[3], "w");
  if (f == NULL) {
    fprintf(stderr, "Error opening %s for write\n", argv[3]);
    exit(1);
  }

  gibr = GeoIPBitReader_new(argv[2]);
  if (gibr == NULL) {
    fprintf(stderr, "Error opening GeoIP Bit Database %s\n", argv[2]);
    exit(1);
  }

	databaseType = GeoIPBitReader_read(gibr, 8);
	for (;;) {
		record = GeoIPBitReader_read(gibr, 5);
		if (record == EOF_FLAG) {
			break;
		} else if (record == NOOP_FLAG) {
			beginIp = endIp;
		} else if (record == VALUE_FLAG) {
			if (GEOIP_COUNTRY_EDITION == databaseType) {
				val = GeoIPBitReader_read(gibr, COUNTRY_RECORD_SIZE);
			} else if (GEOIP_CITY_EDITION_REV0 == databaseType) {
				val = GeoIPBitReader_read(gibr, CITY_RECORD_SIZE);
			}
			if (EXPORT_CSV == exportType) {
				csv_export2(databaseType, beginIp, endIp - 1, val, f);
			} else if (EXPORT_FULL_CSV == exportType) {
				full_csv_export(databaseType, beginIp, endIp - 1, val, f);
			}
			beginIp = endIp;
		} else {
			/* record = netmask - 1 */
			endIp += (1 << (31 - record));
		}
	}

  return 0;
}
