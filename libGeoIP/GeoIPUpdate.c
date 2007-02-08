/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/* GeoIPUpdate.c
 *
 * Copyright (C) 2006 MaxMind LLC
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "GeoIPCity.h"
#include "GeoIP.h"
#include "GeoIPUpdate.h"
#include "GeoIP_internal.h"

#include "global.h"
#include "md5.h"
#include <sys/types.h>
#ifndef WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#else
#include <windows.h>
#include <winsock.h>
#endif
#include <zlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

#define BLOCK_SIZE 1024

const char *GeoIPUpdateHost = "updates.maxmind.com";
const char *GeoIPHTTPRequest = "GET /app/update?license_key=%s&md5=%s HTTP/1.0\nHost: updates.maxmind.com\n\n";
const char *GeoIPHTTPRequestMD5 = "GET /app/update_secure?db_md5=%s&challenge_md5=%s&user_id=%s&edition_id=%s HTTP/1.0\nHost: updates.maxmind.com\n\n";

/* messages */
const char *NoCurrentDB = "%s can't be opened, proceeding to download database\n";
const char *MD5Info = "MD5 Digest of installed database is %s\n";
const char *SavingGzip = "Saving gzip file to %s ... ";
const char *WritingFile = "Writing uncompressed data to %s ...";

/* TODO replace printf with GeoIP_printf - we need someway of having vargs with GeoIP_printf */

const char * GeoIP_get_error_message(int i) {
  switch (i) {
  case GEOIP_NO_NEW_UPDATES:
    return "no new updates";
  case GEOIP_SUCCESS:
    return "Success";
  case GEOIP_LICENSE_KEY_INVALID_ERR:
    return "License Key Invalid";
  case GEOIP_DNS_ERR:
    return "Unable to resolve hostname";
  case GEOIP_NON_IPV4_ERR:
    return "Non - IPv4 address";
  case GEOIP_SOCKET_OPEN_ERR:
    return "Error opening socket";
  case GEOIP_CONNECTION_ERR:
    return "Unable to connect";
  case GEOIP_GZIP_IO_ERR:
    return "Unable to write GeoIP.dat.gz file";
  case GEOIP_TEST_IO_ERR:
    return "Unable to write GeoIP.dat.test file";
  case GEOIP_GZIP_READ_ERR:
    return "Unable to read gzip data";
  case GEOIP_OUT_OF_MEMORY_ERR:
    return "Out of memory error";
  case GEOIP_SOCKET_READ_ERR:
    return "Error reading from socket, see errno";
  case GEOIP_SANITY_OPEN_ERR:
    return "Sanity check GeoIP_open error";
  case GEOIP_SANITY_INFO_FAIL:
    return "Sanity check database_info string failed";
  case GEOIP_SANITY_LOOKUP_FAIL:
    return "Sanity check ip address lookup failed";
  case GEOIP_RENAME_ERR:
    return "Rename error while installing db, check errno";
  case GEOIP_USER_ID_INVALID_ERR:
    return "Invalid userID";
  case GEOIP_PRODUCT_ID_INVALID_ERR:
    return "Invalid product ID or subscription expired";
  case GEOIP_INVALID_SERVER_RESPONSE:
    return "Server returned something unexpected";
  default:
    return "no error";
  }  
}

void GeoIP_printf(void (*f)(char *), const char *str) {
	char * f_str;
	size_t len = strlen(str)+1;
	f_str = malloc(len);
	strncpy(f_str,str,len);
	if (f != NULL)
		(*f)(f_str);
	free(f_str);
}

short int GeoIP_update_database (char * license_key, int verbose, void (*f)( char *)) {
	struct hostent *hostlist;
	int sock;
	char * buf;
	struct sockaddr_in sa;
	int offset = 0, err;
	char * request_uri;
	char * compr;
	unsigned long comprLen;
	FILE *comp_fh, *cur_db_fh, *gi_fh;
	gzFile gz_fh;
	char * file_path_gz, * file_path_test;
	MD5_CONTEXT context;
	unsigned char buffer[1024], digest[16];
	char hex_digest[33] = "00000000000000000000000000000000\0";
	unsigned int i;
	char *f_str;
	GeoIP * gi;
	char * db_info;
	char block[BLOCK_SIZE];
	int block_size = BLOCK_SIZE;
	size_t len;

	_GeoIP_setup_dbfilename();

	/* get MD5 of current GeoIP database file */
	if ((cur_db_fh = fopen (GeoIPDBFileName[GEOIP_COUNTRY_EDITION], "rb")) == NULL) {
		len = strlen(NoCurrentDB) + strlen(GeoIPDBFileName[GEOIP_COUNTRY_EDITION]) - 1;
		f_str = malloc(len);
		snprintf(f_str, len, NoCurrentDB, GeoIPDBFileName[GEOIP_COUNTRY_EDITION]);
		if (f != NULL)
			(*f)(f_str);
		free(f_str);
	} else {
		md5_init(&context);
		while ((len = fread (buffer, 1, 1024, cur_db_fh)) > 0)
			md5_write (&context, buffer, len);
		md5_final (&context);
		memcpy(digest,context.buf,16);
		fclose (cur_db_fh);
		for (i = 0; i < 16; i++) {
			// "%02x" will write 3 chars
			snprintf (&hex_digest[2*i], 3, "%02x", digest[i]);
		}
		len = strlen(MD5Info) + strlen(hex_digest) - 1;
		f_str = malloc(len);
		snprintf(f_str, len, MD5Info, hex_digest);
		if (f != NULL)
			(*f)(f_str);
		free(f_str);
	}

	hostlist = gethostbyname(GeoIPUpdateHost);

	if (hostlist == NULL)
		return GEOIP_DNS_ERR;

	if (hostlist->h_addrtype != AF_INET)
		return GEOIP_NON_IPV4_ERR;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return GEOIP_SOCKET_OPEN_ERR;
	}

	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_port = htons(80);
	memcpy(&sa.sin_addr, hostlist->h_addr_list[0], hostlist->h_length);
	sa.sin_family = AF_INET;

	if (verbose == 1)
		GeoIP_printf(f,"Connecting to MaxMind GeoIP Update server\n");

	/* Download gzip file */
	if (connect(sock, (struct sockaddr *)&sa, sizeof(struct sockaddr))< 0)
		return GEOIP_CONNECTION_ERR;

	request_uri = malloc(sizeof(char) * (strlen(license_key) + strlen(GeoIPHTTPRequest)+36));
	if (request_uri == NULL)
		return GEOIP_OUT_OF_MEMORY_ERR;
	sprintf(request_uri,GeoIPHTTPRequest,license_key, hex_digest);
	send(sock, request_uri, strlen(request_uri),0);
	free(request_uri);

	buf = malloc(sizeof(char) * block_size);
	if (buf == NULL)
		return GEOIP_OUT_OF_MEMORY_ERR;

	if (verbose == 1)
		GeoIP_printf(f,"Downloading gzipped GeoIP Database...\n");

	for (;;) {
		int amt;
		amt = recv(sock, &buf[offset], block_size,0);
		if (amt == 0) {
			break;
		} else if (amt == -1) {
			free(buf);
			return GEOIP_SOCKET_READ_ERR;
		}
		offset += amt;
		buf = realloc(buf, offset+block_size);
		if (buf == NULL)
			return GEOIP_OUT_OF_MEMORY_ERR;
	}

	compr = strstr(buf, "\r\n\r\n") + 4;
	comprLen = offset + buf - compr;

	if (strstr(compr, "License Key Invalid") != NULL) {
		if (verbose == 1)
			GeoIP_printf(f,"Failed\n");
		free(buf);
		return GEOIP_LICENSE_KEY_INVALID_ERR;
	} else if (strstr(compr, "No new updates available") != NULL) {
		free(buf);
		return GEOIP_NO_NEW_UPDATES;
	}

	if (verbose == 1)
		GeoIP_printf(f,"Done\n");

	/* save gzip file */
	file_path_gz = malloc(sizeof(char) * (strlen(GeoIPDBFileName[GEOIP_COUNTRY_EDITION]) + 4));
	if (file_path_gz == NULL)
		return GEOIP_OUT_OF_MEMORY_ERR;
	strcpy(file_path_gz,GeoIPDBFileName[GEOIP_COUNTRY_EDITION]);
	strcat(file_path_gz,".gz");
	if (verbose == 1) {
		len = strlen(SavingGzip) + strlen(file_path_gz) - 1;
		f_str = malloc(len);
		snprintf(f_str, len, SavingGzip,file_path_gz);
		if (f != NULL)
			(*f)(f_str);
		free(f_str);
	}
	comp_fh = fopen(file_path_gz, "wb");

	if(comp_fh == NULL) {
		free(file_path_gz);
		free(buf);
		return GEOIP_GZIP_IO_ERR;
	}

	fwrite(compr, 1, comprLen, comp_fh);
	fclose(comp_fh);
	free(buf);

	if (verbose == 1)
		GeoIP_printf(f,"Done\n");

	if (verbose == 1)
		GeoIP_printf(f,"Uncompressing gzip file ... ");

	/* uncompress gzip file */
	gz_fh = gzopen(file_path_gz, "rb");
	file_path_test = malloc(sizeof(char) * (strlen(GeoIPDBFileName[GEOIP_COUNTRY_EDITION]) + 6));
	if (file_path_test == NULL)
		return GEOIP_OUT_OF_MEMORY_ERR;
	strcpy(file_path_test,GeoIPDBFileName[GEOIP_COUNTRY_EDITION]);
	strcat(file_path_test,".test");
	gi_fh = fopen(file_path_test, "wb");

	if(gi_fh == NULL) {
		free(file_path_test);
		return GEOIP_TEST_IO_ERR;
	}
	for (;;) {
		int amt;
		amt = gzread(gz_fh, block, block_size);
		if (amt == -1) {
			free(file_path_test);
			fclose(gi_fh);
			gzclose(gz_fh);
			return GEOIP_GZIP_READ_ERR;
		}
		if (amt == 0) {
			break;
		}
		fwrite(block,1,amt,gi_fh);
	}
	gzclose(gz_fh);
	unlink(file_path_gz);
	free(file_path_gz);
	fclose(gi_fh);

	if (verbose == 1)
		GeoIP_printf(f,"Done\n");

	if (verbose == 1) {
		f_str = malloc(strlen(WritingFile) + strlen(GeoIPDBFileName[GEOIP_COUNTRY_EDITION]) - 1);
		sprintf(f_str,WritingFile,GeoIPDBFileName[GEOIP_COUNTRY_EDITION]);
		if (f != NULL)
			(*f)(f_str);
		free(f_str);
	}

	/* sanity check */
	gi = GeoIP_open(file_path_test, GEOIP_STANDARD);

	if (verbose == 1)
		GeoIP_printf(f,"Performing santity checks ... ");

	if (gi == NULL) {
		GeoIP_printf(f,"Error opening sanity check database\n");
		return GEOIP_SANITY_OPEN_ERR;
	}

	/* this checks to make sure the files is complete, since info is at the end */
	/* dependent on future databases having MaxMind in info */
	if (verbose == 1)
		GeoIP_printf(f,"database_info  ");
	db_info = GeoIP_database_info(gi);
	if (db_info == NULL) {
		GeoIP_delete(gi);
		if (verbose == 1)
			GeoIP_printf(f,"FAIL\n");
		return GEOIP_SANITY_INFO_FAIL;
	}
	if (strstr(db_info, "MaxMind") == NULL) {
		free(db_info);
		GeoIP_delete(gi);
		if (verbose == 1)
			GeoIP_printf(f,"FAIL\n");
		return GEOIP_SANITY_INFO_FAIL;
	}
	free(db_info);
	if (verbose == 1)
		GeoIP_printf(f,"PASS  ");

	/* this performs an IP lookup test of a US IP address */
	if (verbose == 1)
		GeoIP_printf(f,"lookup  ");
	if (strcmp(GeoIP_country_code_by_addr(gi,"24.24.24.24"), "US") != 0) {
		GeoIP_delete(gi);
		if (verbose == 1)
			GeoIP_printf(f,"FAIL\n");
		return GEOIP_SANITY_LOOKUP_FAIL;
	}
	GeoIP_delete(gi);
	if (verbose == 1)
		GeoIP_printf(f,"PASS\n");

	/* install GeoIP.dat.test -> GeoIP.dat */
	err = rename(file_path_test, GeoIPDBFileName[GEOIP_COUNTRY_EDITION]);
	if (err != 0) {
		GeoIP_printf(f,"GeoIP Install error while renaming file\n");
		return GEOIP_RENAME_ERR;
	}

	if (verbose == 1)
		GeoIP_printf(f,"Done\n");

	return 0;
}

short int GeoIP_update_database_general (char * user_id,char * license_key,char *data_base_type, int verbose,char ** client_ipaddr, void (*f)( char *)) {
	struct hostent *hostlist;
	int sock;
	char * buf;
	struct sockaddr_in sa;
	int offset = 0, err;
	char * request_uri;
	char * compr;
	unsigned long comprLen;
	FILE *comp_fh, *cur_db_fh, *gi_fh;
	gzFile gz_fh;
	char * file_path_gz, * file_path_test;
	MD5_CONTEXT context;
	MD5_CONTEXT context2;
	unsigned char buffer[1024], digest[16] ,digest2[16];
	char hex_digest[33] = "0000000000000000000000000000000\0";
	char hex_digest2[33] = "0000000000000000000000000000000\0";
	unsigned int i;
	char *f_str;
	GeoIP * gi;
	char * db_info;
	char *ipaddress;
	char *geoipfilename;
	char *tmpstr;
	int dbtype;
	int lookupresult = 1;
	char block[BLOCK_SIZE];
	int block_size = BLOCK_SIZE;
	size_t len;
	size_t request_uri_len;

	hostlist = gethostbyname(GeoIPUpdateHost);

	if (hostlist == NULL)
		return GEOIP_DNS_ERR;

	if (hostlist->h_addrtype != AF_INET)
		return GEOIP_NON_IPV4_ERR;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return GEOIP_SOCKET_OPEN_ERR;
	}

	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_port = htons(80);
	memcpy(&sa.sin_addr, hostlist->h_addr_list[0], hostlist->h_length);
	sa.sin_family = AF_INET;
	if (connect(sock, (struct sockaddr *)&sa, sizeof(struct sockaddr))< 0)
		return GEOIP_CONNECTION_ERR;
	request_uri = malloc(sizeof(char) * (strlen(license_key) + strlen(GeoIPHTTPRequestMD5)+1036));
	if (request_uri == NULL)
		return GEOIP_OUT_OF_MEMORY_ERR;

	/* get the file name from a web page using the product id */
	sprintf(request_uri,"GET /app/update_getfilename?product_id=%s HTTP/1.0\nHost: %s\n\n",data_base_type,GeoIPUpdateHost);
	if (verbose == 1) {
		printf("sending request %s \n",request_uri);
	}
	send(sock, request_uri, strlen(request_uri),0); /* send the request */
	free(request_uri);
	buf = malloc(sizeof(char) * (block_size+4));
	if (buf == NULL)
		return GEOIP_OUT_OF_MEMORY_ERR;
	offset = 0;
	for (;;){
		int amt;
		amt = recv(sock, &buf[offset], block_size,0); 
		if (amt == 0){
			break;
		} else if (amt == -1) {
			free(buf);
			return GEOIP_SOCKET_READ_ERR;
		}
		offset += amt;
		buf = realloc(buf, offset + block_size + 4);
	}
	buf[offset] = 0;
	offset = 0;
	tmpstr = strstr(buf, "\r\n\r\n") + 4;
	if (tmpstr[0] == '.' || strchr(tmpstr, '/') != NULL) {
		free(buf);
		return GEOIP_INVALID_SERVER_RESPONSE;
	}
	geoipfilename = _GeoIP_full_path_to(tmpstr);
	free(buf);

	/* print the database product id and the database filename */
	if (verbose == 1){
		printf("database product id %s database file name %s \n",data_base_type,geoipfilename);
	}
	_GeoIP_setup_dbfilename();

	/* get MD5 of current GeoIP database file */
	if ((cur_db_fh = fopen (geoipfilename, "rb")) == NULL) {
		len = strlen(NoCurrentDB) + strlen(geoipfilename) - 1;
		f_str = malloc(len);
		snprintf(f_str, len, NoCurrentDB, geoipfilename);

		if (f != NULL)
			(*f)(f_str);
		free(f_str);
	} else {
		md5_init(&context);
		while ((len = fread (buffer, 1, 1024, cur_db_fh)) > 0)
			md5_write (&context, buffer, len);
		md5_final (&context);
		memcpy(digest,context.buf,16);
		fclose (cur_db_fh);
		for (i = 0; i < 16; i++)
			sprintf (&hex_digest[2*i], "%02x", digest[i]);
		len = strlen(MD5Info) + strlen(hex_digest) - 1;
		f_str = malloc(len);
		snprintf(f_str, len, MD5Info, hex_digest);
		if (f != NULL)
			(*f)(f_str);
		free(f_str);
	}
	if (verbose == 1) {
		printf("MD5 sum of database %s is %s \n",geoipfilename,hex_digest);
	}
	if (client_ipaddr[0] == NULL) {
		/* We haven't gotten our IP address yet, so let's request it */
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			free(geoipfilename);
			return GEOIP_SOCKET_OPEN_ERR;
		}

		memset(&sa, 0, sizeof(struct sockaddr_in));
		sa.sin_port = htons(80);
		memcpy(&sa.sin_addr, hostlist->h_addr_list[0], hostlist->h_length);
		sa.sin_family = AF_INET;

		if (verbose == 1)
			GeoIP_printf(f,"Connecting to MaxMind GeoIP Update server\n");

		/* Download gzip file */
		if (connect(sock, (struct sockaddr *)&sa, sizeof(struct sockaddr))< 0) {
			free(geoipfilename);
			return GEOIP_CONNECTION_ERR;
		}
		request_uri = malloc(sizeof(char) * (strlen(license_key) + strlen(GeoIPHTTPRequestMD5)+1036));
		if (request_uri == NULL) {
			free(geoipfilename);
			return GEOIP_OUT_OF_MEMORY_ERR;
		}

		/* get client ip address from MaxMind web page */
		sprintf(request_uri,"GET /app/update_getipaddr HTTP/1.0\nHost: %s\n\n",GeoIPUpdateHost);
		send(sock, request_uri, strlen(request_uri),0); /* send the request */
		if (verbose == 1) {
			printf("sending request %s", request_uri);
		}
		free(request_uri);
		buf = malloc(sizeof(char) * (block_size+1));
		if (buf == NULL) {
			free(geoipfilename);
			return GEOIP_OUT_OF_MEMORY_ERR;
		}
		offset = 0;

		for (;;){
			int amt;
			amt = recv(sock, &buf[offset], block_size,0); 
			if (amt == 0) {
				break;
			} else if (amt == -1) {
				free(buf);
				return GEOIP_SOCKET_READ_ERR;
			}
			offset += amt;
			buf = realloc(buf, offset+block_size+1);
		}

		buf[offset] = 0;
		offset = 0;
		ipaddress = strstr(buf, "\r\n\r\n") + 4; /* get the ip address */
		ipaddress = malloc(strlen(strstr(buf, "\r\n\r\n") + 4)+5);
		strcpy(ipaddress,strstr(buf, "\r\n\r\n") + 4);
		client_ipaddr[0] = ipaddress;
		if (verbose == 1) {
			printf("client ip address: %s\n",ipaddress);
		}
		free(buf);
		close(sock);
	}

	ipaddress = client_ipaddr[0];

	/* make a md5 sum of ip address and license_key and store it in hex_digest2 */
	request_uri_len = sizeof(char) * 2036;
	request_uri = malloc(request_uri_len);
	md5_init(&context2);
	md5_write (&context2, license_key, 12);//add license key to the md5 sum
	md5_write (&context2, ipaddress, strlen(ipaddress));//add ip address to the md5 sum
	md5_final (&context2);
	memcpy(digest2,context2.buf,16);
	for (i = 0; i < 16; i++)
		snprintf (&hex_digest2[2*i], 3, "%02x", digest2[i]);// change the digest to a hex digest
	if (verbose == 1) {
		printf("md5sum of ip address and license key is %s \n",hex_digest2);
	}

	/* send the request using the user id,product id, 
	 * md5 sum of the prev database and 
	 * the md5 sum of the license_key and ip address */
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return GEOIP_SOCKET_OPEN_ERR;
	}
	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_port = htons(80);
	memcpy(&sa.sin_addr, hostlist->h_addr_list[0], hostlist->h_length);
	sa.sin_family = AF_INET;
	if (connect(sock, (struct sockaddr *)&sa, sizeof(struct sockaddr))< 0)
		return GEOIP_CONNECTION_ERR;
	snprintf(request_uri, request_uri_len, GeoIPHTTPRequestMD5,hex_digest,hex_digest2,user_id,data_base_type);
	send(sock, request_uri, strlen(request_uri),0);
	if (verbose == 1) {
		printf("sending request %s\n",request_uri);
	}

	free(request_uri);

	offset = 0;
	buf = malloc(sizeof(char) * block_size);
	if (buf == NULL)
		return GEOIP_OUT_OF_MEMORY_ERR;

	if (verbose == 1)
		GeoIP_printf(f,"Downloading gzipped GeoIP Database...\n");

	for (;;) {
		int amt;
		amt = recv(sock, &buf[offset], block_size,0);

		if (amt == 0) {
			break;
		} else if (amt == -1) {
			free(buf);
			return GEOIP_SOCKET_READ_ERR;
		}
		offset += amt;
		buf = realloc(buf, offset+block_size);
		if (buf == NULL)
			return GEOIP_OUT_OF_MEMORY_ERR;
	}

	compr = strstr(buf, "\r\n\r\n") + 4;
	comprLen = offset + buf - compr;

	if (strstr(compr, "License Key Invalid") != NULL) {
		if (verbose == 1)
			GeoIP_printf(f,"Failed\n");
		free(buf);
		return GEOIP_LICENSE_KEY_INVALID_ERR;
	} else if (strstr(compr, "No new updates available") != NULL) {
		free(buf);
		printf("%s is up to date, no updates required\n", geoipfilename);
		return GEOIP_NO_NEW_UPDATES;
	} else if (strstr(compr, "Invalid UserId") != NULL){
		free(buf);
		return GEOIP_USER_ID_INVALID_ERR;
	} else if (strstr(compr, "Invalid product ID or subscription expired") != NULL){
		free(buf);
		return GEOIP_PRODUCT_ID_INVALID_ERR;
	}

	if (verbose == 1)
		GeoIP_printf(f,"Done\n");

	printf("Updating %s\n", geoipfilename);

	/* save gzip file */
	file_path_gz = malloc(sizeof(char) * (strlen(geoipfilename) + 4));

	if (file_path_gz == NULL)
		return GEOIP_OUT_OF_MEMORY_ERR;
	strcpy(file_path_gz,geoipfilename);
	strcat(file_path_gz,".gz");
	if (verbose == 1) {
		len = strlen(SavingGzip) + strlen(file_path_gz) - 1;
		f_str = malloc(len);
		snprintf(f_str,len,SavingGzip,file_path_gz);
		if (f != NULL)
			(*f)(f_str);
		free(f_str);
	}
	comp_fh = fopen(file_path_gz, "wb");

	if(comp_fh == NULL) {
		free(file_path_gz);
		free(buf);
		return GEOIP_GZIP_IO_ERR;
	}

	fwrite(compr, 1, comprLen, comp_fh);
	fclose(comp_fh);
	free(buf);

	if (verbose == 1) {
		printf("download data to a gz file named %s \n",file_path_gz);
	}
	if (verbose == 1)
		GeoIP_printf(f,"Done\n");

	if (verbose == 1)
		GeoIP_printf(f,"Uncompressing gzip file ... ");

	file_path_test = malloc(sizeof(char) * (strlen(GeoIPDBFileName[GEOIP_COUNTRY_EDITION]) + 6));
	if (file_path_test == NULL) {
		free(file_path_gz);
		return GEOIP_OUT_OF_MEMORY_ERR;
	}
	strcpy(file_path_test,GeoIPDBFileName[GEOIP_COUNTRY_EDITION]);
	strcat(file_path_test,".test");
	gi_fh = fopen(file_path_test, "wb");
	if(gi_fh == NULL) {
		free(file_path_test);
		free(file_path_gz);
		return GEOIP_TEST_IO_ERR;
	}
	/* uncompress gzip file */
	offset = 0;
	gz_fh = gzopen(file_path_gz, "rb");
	for (;;) {
		int amt;
		amt = gzread(gz_fh, block, block_size);
		if (amt == -1) {
			free(file_path_gz);
			free(file_path_test);
			gzclose(gz_fh);
			fclose(gi_fh);
			return GEOIP_GZIP_READ_ERR;
		}
		if (amt == 0) {
			break;
		}
		fwrite(block,1,amt,gi_fh);
	}
	gzclose(gz_fh);
	unlink(file_path_gz);
	free(file_path_gz);
	fclose(gi_fh);

	if (verbose == 1)
		GeoIP_printf(f,"Done\n");

	if (verbose == 1) {
		len = strlen(WritingFile) + strlen(geoipfilename) - 1;
		f_str = malloc(len);
		snprintf(f_str,len,WritingFile,geoipfilename);
		free(f_str);
	}

	/* sanity check */
	gi = GeoIP_open(file_path_test, GEOIP_STANDARD);

	if (verbose == 1)
		GeoIP_printf(f,"Performing santity checks ... ");

	if (gi == NULL) {
		GeoIP_printf(f,"Error opening sanity check database\n");
		return GEOIP_SANITY_OPEN_ERR;
	}


	/* get the database type */
	dbtype = GeoIP_database_edition(gi);
	if (verbose == 1) {
		printf("Database type is %d\n",dbtype);
	}

	/* this checks to make sure the files is complete, since info is at the end
		 dependent on future databases having MaxMind in info (ISP and Organization databases currently don't have info string */

	if ((dbtype != GEOIP_ISP_EDITION)&&
			(dbtype != GEOIP_ORG_EDITION)) {
		if (verbose == 1)
			GeoIP_printf(f,"database_info  ");
		db_info = GeoIP_database_info(gi);
		if (db_info == NULL) {
			GeoIP_delete(gi);
			if (verbose == 1)
				GeoIP_printf(f,"FAIL null\n");
			return GEOIP_SANITY_INFO_FAIL;
		}
		if (strstr(db_info, "MaxMind") == NULL) {
			free(db_info);
			GeoIP_delete(gi);
			if (verbose == 1)
				GeoIP_printf(f,"FAIL maxmind\n");
			return GEOIP_SANITY_INFO_FAIL;
		}
		free(db_info);
		if (verbose == 1)
			GeoIP_printf(f,"PASS  ");
	}

	/* this performs an IP lookup test of a US IP address */
	if (verbose == 1)
		GeoIP_printf(f,"lookup  ");
	if (dbtype == GEOIP_NETSPEED_EDITION) {
		int netspeed = GeoIP_id_by_name(gi,"24.24.24.24");
		lookupresult = 0;
		if (netspeed == GEOIP_CABLEDSL_SPEED){
			lookupresult = 1;
		}
	}
	if (dbtype == GEOIP_COUNTRY_EDITION) {
		/* if data base type is country then call the function
		 * named GeoIP_country_code_by_addr */
		lookupresult = 1;
		if (strcmp(GeoIP_country_code_by_addr(gi,"24.24.24.24"), "US") != 0) {
			lookupresult = 0;
		}
		if (verbose == 1) {
			GeoIP_printf(f,"testing GEOIP_COUNTRY_EDITION\n");
		}
	}
	if (dbtype == GEOIP_REGION_EDITION_REV1) {
		/* if data base type is region then call the function
		 * named GeoIP_region_by_addr */
		GeoIPRegion *r = GeoIP_region_by_addr(gi,"24.24.24.24");
		lookupresult = 0;
		if (r != NULL) {
			lookupresult = 1;
			free(r);
		}
		if (verbose == 1) {
			GeoIP_printf(f,"testing GEOIP_REGION_EDITION\n");
		}
	}
	if (dbtype == GEOIP_CITY_EDITION_REV1) {
		/* if data base type is city then call the function
		 * named GeoIP_record_by_addr */
		GeoIPRecord *r = GeoIP_record_by_addr(gi,"24.24.24.24");
		lookupresult = 0;
		if (r != NULL) {
			lookupresult = 1;
			free(r);
		}
		if (verbose == 1) {
			GeoIP_printf(f,"testing GEOIP_CITY_EDITION\n");
		}
	}
	if ((dbtype == GEOIP_ISP_EDITION)||
			(dbtype == GEOIP_ORG_EDITION)) {
		/* if data base type is isp or org then call the function
		 * named GeoIP_org_by_addr */
		GeoIPRecord *r = (GeoIPRecord*)GeoIP_org_by_addr(gi,"24.24.24.24");
		lookupresult = 0;
		if (r != NULL) {
			lookupresult = 1;
			free(r);
		}
		if (verbose == 1) {
			if (dbtype == GEOIP_ISP_EDITION) {
				GeoIP_printf(f,"testing GEOIP_ISP_EDITION\n");
			}
			if (dbtype == GEOIP_ORG_EDITION) {
				GeoIP_printf(f,"testing GEOIP_ORG_EDITION\n");
			}
		}
	}
	if (lookupresult == 0) {
		GeoIP_delete(gi);
		if (verbose == 1)
			GeoIP_printf(f,"FAIL\n");
		return GEOIP_SANITY_LOOKUP_FAIL;
	}
	GeoIP_delete(gi);
	if (verbose == 1)
		GeoIP_printf(f,"PASS\n");

	/* install GeoIP.dat.test -> GeoIP.dat */
	err = rename(file_path_test, geoipfilename);
	if (err != 0) {
		GeoIP_printf(f,"GeoIP Install error while renaming file\n");
		return GEOIP_RENAME_ERR;
	}

	if (verbose == 1)
		GeoIP_printf(f,"Done\n");
	free(geoipfilename);
	return 0;
}
