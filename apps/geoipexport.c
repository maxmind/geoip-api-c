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

void usage() {
  fprintf(stderr,"Usage: geoipexport binary_file csv_file1 csv_file2 ...\n");
}

void print_netblock (GeoIP *gi, FILE *f, int ipnum, int depth, int val) {
  if (gi->databaseType == GEOIP_COUNTRY_EDITION) {
    fprintf(f, "\"%d\",\"%d\",\"%s\"\r\n", ipnum, ipnum + (1 << (32 - depth)) - 1, GeoIP_country_code[val - gi->databaseSegments[0]]);
  } else {

  }
}

/* print out binary tree using depth first search */
void export_netblock (GeoIP* gi, FILE *f, int ipnum, int depth, int offset) {
  unsigned char *cache_buf = NULL;
  int i, j;
  unsigned int x[2];

  printf("offset = %d\n", offset);

  cache_buf = gi->cache + (long)RECORD_LENGTH * 2 * offset;
  for (i = 0; i < 2; i++) {
    x[i] = 0;
    for (j = 0; j < RECORD_LENGTH; j++) {
      x[i] += (cache_buf[i*RECORD_LENGTH + j] << (j * 8));
    }
  }
  depth++;

  /* go left down tree*/
  if (x[1] >= gi->databaseSegments[0]) {
    print_netblock(gi,f,ipnum,depth,x[1]);
  } else {
    export_netblock(gi,f,ipnum,depth,x[1]);
  }

  /* go right down tree */
  ipnum += (1 << (32 - depth));
  if (x[0] >= gi->databaseSegments[0]) {
    print_netblock(gi,f,ipnum,depth,x[0]);
  } else {
    export_netblock(gi,f,ipnum,depth,x[0]);
  }
}

int main (int argc, char *argv[]) {
  FILE *f;
  GeoIP *gi;

  if (argc < 3) {
    usage();
    exit(1);
  }

  f = fopen(argv[2], "w");
  if (f == NULL) {
    fprintf(stderr, "Error opening %s for write\n", argv[2]);
    exit(1);
  }
  gi = GeoIP_open(argv[1], GEOIP_MEMORY_CACHE);
  if (gi == NULL) {
    fprintf(stderr, "Error opening GeoIP Database %s\n", argv[1]);
    exit(1);
  }
  export_netblock(gi,f,0,0,0);
  return 0;
}

