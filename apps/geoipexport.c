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

void usage() {
  fprintf(stderr,"Usage: geoipexport bit_file csv_file1 csv_file2 ...\n");
}

char * _num_to_addr (unsigned long num) {
	char * addr;
	asprintf(&addr, "%u.%u.%u.%u",
					 (int)floor(num/16777216),
					 ((int)floor(num/65536)) % 256,
					 ((int)floor(num/256)) % 256,
					 num % 256);
	return addr;
}

void full_csv_export (int databaseType, int beginIp, int endIp, int val, FILE *out) {
	fprintf(out, "\"%s\",\"%s\",\"%u\",\"%u\",\"%s\",\"%s\"\n",
					_num_to_addr(beginIp), _num_to_addr(endIp), beginIp, endIp,
					GeoIP_country_code[val], GeoIP_country_name[val]);

}

int main (int argc, char *argv[]) {
  FILE *f;
  GeoIPBitReader *gibr;
	int databaseType, record, val;
	int beginIp = 0, endIp = 0;

  if (argc < 3) {
    usage();
    exit(1);
  }

  f = fopen(argv[2], "w");
  if (f == NULL) {
    fprintf(stderr, "Error opening %s for write\n", argv[2]);
    exit(1);
  }

  gibr = GeoIPBitReader_new(argv[1]);
  if (gibr == NULL) {
    fprintf(stderr, "Error opening GeoIP Bit Database %s\n", argv[1]);
    exit(1);
  }

	databaseType = GeoIPBitReader_read(gibr, 8);
	printf("databaseType = %d\n", databaseType);
	for (;;) {
		record = GeoIPBitReader_read(gibr, 5);
		if (record == EOF_FLAG) {
			break;
		} else if (record == NOOP_FLAG) {
			beginIp = endIp;
		} else if (record == VALUE_FLAG) {
			/* assume databaseType == country for now */
			val = GeoIPBitReader_read(gibr, 8);
			/* printf("country record (%d, %d) -> %d\n", beginIp, endIp-1, val); */
			full_csv_export(databaseType, beginIp, endIp - 1, val, f);
		} else {
			/* record = netmask - 1 */
			endIp += (1 << (31 - record));
		}
	}

  return 0;
}
