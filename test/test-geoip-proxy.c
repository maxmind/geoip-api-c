/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/* test-geoip-proxy.c
 *
 * Copyright (C) 2003 MaxMind LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <GeoIP.h>

int main () {
  FILE *f;
  char ipAddress[30];
  int expected;
  int returnedValue;
	GeoIP * gi;
	int failed = 0;
	int test_num = 1;

	int i;
	for (i = 0; i < 2; ++i) {
		if (0 == i) {
			gi = GeoIP_open("../data/GeoIPProxy.dat", GEOIP_STANDARD);
		} else {
			gi = GeoIP_open("../data/GeoIPProxy.dat", GEOIP_MEMORY_CACHE);
		}

		if (gi == NULL) {
			fprintf(stderr, "Error opening database\n");
			exit(1);
		}

		f = fopen("proxy_test.txt","r");

		while (fscanf(f, "%s", ipAddress) != EOF) {
			fscanf(f, "%d", &expected);
			returnedValue = GeoIP_id_by_addr(gi,ipAddress);
			if (returnedValue != expected) {
				fprintf(stderr,"Test addr %d for %s failed, got %d, expected %d\n",test_num,ipAddress,returnedValue,expected);
				failed = 1;
			}
			returnedValue = GeoIP_id_by_name(gi,ipAddress);
			if (returnedValue != expected) {
				fprintf(stderr,"Test addr %d for %s failed, got %d, expected %d\n",test_num,ipAddress,returnedValue,expected);
				failed = 1;
			}
			test_num++;
		}
		fclose(f);
		GeoIP_delete(gi);
	}
	return failed;
}
