/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/* test-geoip.c
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

#include <GeoIP.h>

const int numIps = 70;
const int numLoops = 1000;

int main () {
  FILE *f;
  char ipAddress[numIps][30];
  char expectedCountry[numIps][3];
  char expectedCountry3[numIps][4];
  const char * returnedCountry;
	GeoIP * gi;
	int failed = 0;
	int test_num = 0;
	int i, j;
	/*gi = GeoIP_open("../data/GeoIP.dat", GEOIP_MEMORY_CACHE);*/
	gi = GeoIP_open("../data/GeoIP.dat", GEOIP_STANDARD);

	if (gi == NULL) {
		fprintf(stderr, "Error opening database\n");
		exit(1);
	}

  f = fopen("country_test.txt","r");

	while (fscanf(f, "%s", ipAddress[test_num]) != EOF) {
		fscanf(f, "%s", expectedCountry[test_num]);
		fscanf(f, "%s", expectedCountry3[test_num]);
		test_num++;
	}

	for (i = 0; i < numLoops; i++) {
		for (j =0; j < numIps; j++) {
			returnedCountry = GeoIP_country_code_by_addr(gi,ipAddress[j]);
			if (strcmp(returnedCountry, expectedCountry[j]) != 0) {
				fprintf(stderr,"Test for %s failed, got %s, expected %s\n",ipAddress[j],returnedCountry,expectedCountry[j]);
				failed = 1;
			}
		}
	}

	printf("Ran %d lookups\n", numIps * numLoops);

	fclose(f);
	GeoIP_delete(gi);
	return failed;
}
