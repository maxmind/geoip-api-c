/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/* geoipexportlocations.c
 *
 * Copyright (C) 2003 MaxMind, LLC
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
#include "GeoIP.h"
#include "GeoIPCity.h"

int main (int argc, char *argv[]) {
  GeoIP *gi;
  GeoIPRecord *gir;
  int record_iter;
  int record_id;
  /* note we must use GEOIP_STANDARD here */
  gi = GeoIP_open_type(GEOIP_CITY_EDITION_REV1, GEOIP_STANDARD);

  if (strcmp(argv[1],"-l") == 0) {
    printf("lookup of %s resulted in record ID %d\n",argv[1],GeoIP_record_id_by_addr(gi, argv[2]));
    return 0;
  }

  record_iter = GeoIP_init_record_iter(gi);
  while (1) {
    record_id = record_iter;
    GeoIP_next_record(gi, &gir, &record_iter);
    if (gir != NULL) {
      printf("\"%d\",\"%s\",\"%s\",\"%s\",\"%s\",\"%.4f\",\"%.4f\",\"%d\",\"%d\"\n",
	     record_id, gir->country_code, gir->region, gir->city, gir->postal_code,
	     gir->latitude, gir->longitude, gir->dma_code, gir->area_code);
    } else {
      /* eof reached (or file error) */
      break;
    }
  }
  return 0;
}
