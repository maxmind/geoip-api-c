/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/* test-geoip.c
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

#include <GeoIP.h>

int main () {
  FILE *f;
  char ipAddress[30];
  char expectedCountry[3];
  char expectedCountry3[4];
  const char * returnedCountry;
	GeoIP * gi;
	int failed = 0;
	int test_num = 1;
	gi = GeoIP_open("../data/GeoIP.dat", GEOIP_MEMORY_CACHE);

	if (gi == NULL) {
		fprintf(stderr, "Error opening database\n");
		exit(1);
	}

  /* make sure GeoIP deals with invalid query gracefully */
  returnedCountry = GeoIP_country_code_by_addr(gi,NULL);
	if (returnedCountry != NULL) {
		fprintf(stderr,"Invalid Query test failed, got non NULL, expected NULL\n");
		failed = 1;
	}
/* commented out b/c March 2002 demo database returns 'US'
  returnedCountry = GeoIP_country_code_by_addr(gi,"");
	if (strcmp(returnedCountry, "--") != 0) {
		fprintf(stderr,"Invalid Query test failed, got %s, expected %s\n",returnedCountry,"--");
		failed = 1;
	}
  returnedCountry = GeoIP_country_code_by_addr(gi,"foo");
	if (strcmp(returnedCountry, "--") != 0) {
		fprintf(stderr,"Invalid Query test failed, got %s, expected %s\n",returnedCountry,"--");
		failed = 1;
	}
*/
  returnedCountry = GeoIP_country_code_by_name(gi,NULL);
	if (returnedCountry != NULL) {
		fprintf(stderr,"Invalid Query test failed, got non NULL, expected NULL\n");
		failed = 1;
	}

  f = fopen("country_test.txt","r");

  while (fscanf(f, "%s", ipAddress) != EOF) {
    fscanf(f, "%s", expectedCountry);
    fscanf(f, "%s", expectedCountry3);
    returnedCountry = GeoIP_country_code_by_addr(gi,ipAddress);
		if (returnedCountry == NULL || strcmp(returnedCountry, expectedCountry) != 0) {
			fprintf(stderr,"Test addr %d for %s failed, got %s, expected %s\n",test_num,ipAddress,returnedCountry,expectedCountry);
			failed = 1;
		}
    returnedCountry = GeoIP_country_code_by_name(gi,ipAddress);
		if (returnedCountry == NULL || strcmp(returnedCountry, expectedCountry) != 0) {
			fprintf(stderr,"Test name %d for %s failed, got %s, expected %s\n",test_num,ipAddress,returnedCountry,expectedCountry);
			failed = 1;
		}
    returnedCountry = GeoIP_country_code3_by_addr(gi,ipAddress);
		if (returnedCountry == NULL || strcmp(returnedCountry, expectedCountry3) != 0) {
			fprintf(stderr,"Test addr %d for %s failed, got %s, expected %s\n",test_num,ipAddress,returnedCountry,expectedCountry);
			failed = 1;
		}
    returnedCountry = GeoIP_country_code3_by_name(gi,ipAddress);
		if (returnedCountry == NULL || strcmp(returnedCountry, expectedCountry3) != 0) {
			fprintf(stderr,"Test name %d for %s failed, got %s, expected %s\n",test_num,ipAddress,returnedCountry,expectedCountry);
			failed = 1;
		}
		test_num++;
  }

  f = fopen("country_test2.txt","r");
  while (fscanf(f, "%s", ipAddress) != EOF) {
    fscanf(f, "%s", expectedCountry);
    returnedCountry = GeoIP_country_code_by_addr(gi,ipAddress);
		if (returnedCountry == NULL || strcmp(returnedCountry, expectedCountry) != 0) {
			fprintf(stderr,"Test addr %d %s failed, got %s, expected %s\n",test_num,ipAddress,returnedCountry,expectedCountry);
			failed = 1;
		}
		test_num++;
  }

  f = fopen("country_test_name.txt","r");
  while (fscanf(f, "%s", ipAddress) != EOF) {
    fscanf(f, "%s", expectedCountry);
    returnedCountry = GeoIP_country_code_by_name(gi,ipAddress);
		if (returnedCountry == NULL || strcmp(returnedCountry, expectedCountry) != 0) {
			fprintf(stderr,"Test addr %d %s failed, got %s, expected %s\n",test_num,ipAddress,returnedCountry,expectedCountry);
			failed = 1;
		}
		test_num++;
  }

	fclose(f);
	GeoIP_delete(gi);
	return failed;
}
