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

#include "GeoIP.h"
#include "GeoIPUpdate.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __linux__
#include <getopt.h>
#endif
#include <ctype.h>

#define LICENSE_KEY_TOKEN "LicenseKey"
#define LICENSE_KEY_LENGTH 12

const char *GeoIPConfFile = "GeoIP.conf";

void usage() {
  fprintf(stderr,"Usage: geoipupdate [-hv] [-f license_file]\n");
}

void my_printf(char * str) {
	printf(str);
	free(str);
}

int main (int argc, char *argv[]) {
  int verbose = 0;
  char * license_file = NULL;
	FILE * license_fh;
	int n = 40;
	int line_index = 0;
	unsigned char *lineptr = malloc(sizeof(char) * n);
	char *a_license_key_str, *a_ptr;
	char *the_license_key_str = "";
	char c;
	int err;

	opterr = 0;

	while ((c = getopt (argc, argv, "hvf:")) != -1)
    switch (c) {
		case 'h':
			usage();
			exit(0);
		case 'v':
			verbose = 1;
		case 'f':
			license_file = optarg;
			break;
		case '?':
			if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr,
								 "Unknown option character `\\x%x'.\n",
								 optopt);
			usage();
			exit(1);
		default:
			abort();
		}

	if (license_file == NULL) {
		license_file = malloc(sizeof(char) * (strlen(SYSCONFDIR)+strlen(GeoIPConfFile)+2));
		license_file[0] = '\0';
		strcat(license_file, SYSCONFDIR);
		strcat(license_file, "/");
		strcat(license_file, GeoIPConfFile);
	}

  license_fh = fopen(license_file,"r");
	if (license_fh == NULL) {
		fprintf(stderr,"Error opening GeoIP Configuration file %s\n",license_file);
		exit(1);
	}

	if (verbose == 1)
		printf("Opened License file %s\n", license_file);

	do {
		c = fgetc(license_fh);
		if (line_index >= n) {
			n += 20;
			lineptr = realloc(lineptr, n);
		}
		if (c == 10 || c == EOF) {
			lineptr[line_index++] = '\0';
			line_index = 0;
			if (lineptr[0] == '#')
				continue;
			a_license_key_str = strstr(lineptr, LICENSE_KEY_TOKEN);
			if (a_license_key_str != NULL) {
				a_ptr = a_license_key_str;
				a_ptr += strlen(LICENSE_KEY_TOKEN) + 1;
				while (a_ptr[0] == ' ') {
					a_ptr++;
				}
				the_license_key_str = malloc(sizeof(char) * (LICENSE_KEY_LENGTH + 1));
				strncpy(the_license_key_str, a_ptr, LICENSE_KEY_LENGTH);
				the_license_key_str[LICENSE_KEY_LENGTH] = '\0';
				break;
			}
		} else {
			lineptr[line_index++] = c;
		}
	} while (c != EOF);

	free(lineptr);

	fclose(license_fh);

	if (verbose == 1)
		printf("Read in license key %s\n", the_license_key_str);

	err = GeoIP_update_database(the_license_key_str, verbose, &my_printf);

	if (err == GEOIP_NO_NEW_UPDATES) {
		fprintf(stderr,"GeoIP Database up to date\n");
	} else if (err == GEOIP_LICENSE_KEY_INVALID_ERR) {
		fprintf(stderr,"Invalid License Key in %s - Please visit http://maxmind.com/ for a subscription\n",license_file);
	} else if (err < 0) {
		fprintf(stderr,"Received Error %d when attempting to update GeoIP Database\n",err);
	} else {
		fprintf(stderr,"Updated database.\n");
	}

	free(the_license_key_str);
	exit(0);
}
