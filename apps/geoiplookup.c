/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/* geoiplookup.c
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

void usage() {
	fprintf(stderr,"Usage: geoiplookup [-lv] <ipaddress|hostname>\n");
}

int main (int argc, char *argv[]) {
	int long_format = 0;
	const char * hostname;
	const char * result;
	char * db_info;
	GeoIP * gi;

	if (argv[1] == NULL) {
		usage();
		exit(1);
	}
	if (strcmp(argv[1],"-l") == 0) {
		long_format = 1;
		if (argv[2] == NULL) {
			usage();
			exit(1);
		}
		hostname = argv[2];
	} else if (strcmp(argv[1],"-v") == 0) {
		gi = GeoIP_new(GEOIP_STANDARD);
		db_info = GeoIP_database_info(gi);
		printf("%s\n",db_info);
		free(db_info);
		GeoIP_delete(gi);
		exit(0);
	} else {
		hostname = argv[1];
	}
	gi = GeoIP_new(GEOIP_STANDARD);
	if (gi == NULL) {
		fprintf(stderr,"Error opening database\n");
		exit(1);
	}
	if (long_format == 1) {
		result = GeoIP_country_name_by_name(gi, hostname);
	} else {
		result = GeoIP_country_code_by_name(gi, hostname);
	}
	GeoIP_delete(gi);
	if (result != NULL) {
		printf("%s\n",result);
	} else {
		printf("IP Address not found\n");
	}
	return 0;
}
