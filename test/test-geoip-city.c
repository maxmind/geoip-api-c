/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/* test-geoip-full.c
 *
 * Copyright (C) 2002 MaxMind.com
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

#include "GeoIP.h"
#include "GeoIPCity.h"

int main (int argc, char* argv[]) {
	FILE *f;
	GeoIP * gi;
	GeoIPRecord * gir;
	int generate = 0;
	char host[50];

	if (argc == 2)
		if (!strcmp(argv[1],"gen"))
			generate = 1;

	gi = GeoIP_open("../data/GeoIPFull.dat", GEOIP_MEMORY_CACHE);

	if (gi == NULL) {
		fprintf(stderr, "Error opening database\n");
		exit(1);
	}

	f = fopen("city_test.txt","r");

	if (f == NULL) {
		fprintf(stderr, "Error opening city_test.txt\n");
		exit(1);
	}

	while (fscanf(f, "%s", host) != EOF) {
		gir = GeoIP_record_by_name (gi, (const char *)host);

		if (gir != NULL) {
			printf("%s\t%s\t%s\t%s\t%s\t%f\t%f\n", host,
					 gir->country_code,
					 gir->region,
					 gir->city,
					 gir->postal_code,
					 gir->latitude,
					 gir->longitude);
			GeoIPRecord_delete(gir);
		}
	}

	GeoIP_delete(gi);
}
