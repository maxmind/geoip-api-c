/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/* test-geoip-netspeed.c
 *
 * Copyright (C) 2004 MaxMind LLC
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

int main (int argc, char* argv[]) {
	FILE *f;
	GeoIP * gi;
	int netspeed;
	char host[50];

	gi = GeoIP_open("../data/GeoIPNetSpeed.dat", GEOIP_STANDARD);

	if (gi == NULL) {
		fprintf(stderr, "Error opening database\n");
		exit(1);
	}

	f = fopen("netspeed_test.txt","r");

	if (f == NULL) {
		fprintf(stderr, "Error opening netspeed_test.txt\n");
		exit(1);
	}

	while (fscanf(f, "%s", host) != EOF) {
		netspeed = GeoIP_id_by_name (gi, (const char *)host);

                  printf("%s\t%d\n", host, netspeed);
	}

	GeoIP_delete(gi);
}
