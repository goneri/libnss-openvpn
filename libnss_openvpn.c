/*  
 *  Copyright © 2012-2014 Gonéri Le Bouder
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  This file incorporates some code from these modules:
 *    nss-mdns, © 2004 Lennart Poettering.
 *    nss-gw-name, © 2010 Joachim Breitner.
 */

#define OPENVPN_STATUS_FILE "/var/run/openvpn.server.status"
#include <stdio.h>
#include <string.h>
#include <nss.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>

#define BUFLEN 256


#define ALIGN(idx) do { \
    if (idx % sizeof(void*)) \
    idx += (sizeof(void*) - idx % sizeof(void*)); /* Align on 32 bit boundary */ \
} while(0)


enum nss_status _nss_openvpn_gethostbyname_r (
        const char *name,
        struct hostent *result,
        char *buffer,
        size_t buflen,
        int *errnop,
        int *h_errnop) {


    FILE * fh = fopen(OPENVPN_STATUS_FILE, "r");

    char * t;
    char * current_ip;
    char * current_hostname;
    size_t idx, astart;

    char strbuf[BUFLEN];

    if (!fh || strcmp(name+strlen(name)-4, ".vpn") != 0) {
        if (fh) {
            fclose(fh);
        }
        *errnop = EINVAL;
        *h_errnop = NO_RECOVERY;
        return NSS_STATUS_UNAVAIL;
    }

    int in = 0;
    while (fgets(strbuf, BUFLEN, fh)) {

        if (strncmp(strbuf, "Virtual Address", 10) == 0) {
            in = 1;
            continue;
        }
        if (strncmp(strbuf, "GLOBAL STATS", 10) == 0) {
            in = 0;
            continue;
        }

        if (!in) {
            continue;
        }

        current_ip=strbuf;
        t = strchr(current_ip, ',');
        if (t == NULL) {
            continue;
        }
        *t = 0;

        current_hostname=current_ip+strlen(current_ip)+1;
        t = strchr(current_hostname, ',');
        if (t == NULL) {
            continue;
        }
        *t = 0;

        if (strlen(current_hostname) + 5 > BUFLEN) {
            continue;
        }
        strcpy(t, ".vpn\0");

        // Is it the host I'm looking for?
        if (strcmp(current_hostname, name)!=0) {
            continue;
        }

        *((char**) buffer) = NULL;
        result->h_aliases = (char**) buffer;
        idx = sizeof(char*);

        /* Official name */
        strcpy(buffer+idx, name);
        result->h_name = buffer+idx;
        idx += strlen(name)+1;
        ALIGN(idx);


        result->h_addrtype = AF_INET;
        result->h_length = sizeof(uint32_t);

        struct in_addr addr;

        inet_pton(AF_INET, current_ip, &addr);

        char a[2000];
        inet_ntop(AF_INET, &addr.s_addr, a, 2000);
        //fprintf(stdout, "IP found: %s\n",a);

        astart = idx;
        memcpy(buffer+astart, &addr.s_addr, sizeof(uint32_t));
        idx += sizeof(uint32_t);

        result->h_addr_list = (char**)(buffer + idx);
        result->h_addr_list[0] = buffer + astart;
        result->h_addr_list[1] = NULL;

        fclose(fh);

        return NSS_STATUS_SUCCESS;
    }

    fclose(fh);

    *errnop = EINVAL;
    *h_errnop = NO_RECOVERY;
    return NSS_STATUS_UNAVAIL;

}

enum nss_status _nss_openvpn_gethostbyname2_r(
	const char *name,
	int af,
	struct hostent * result,
	char *buffer,
	size_t buflen,
	int *errnop,
	int *h_errnop) {

	if (af != AF_INET) {
		*errnop = EAGAIN;
		*h_errnop = NO_RECOVERY;
		return NSS_STATUS_TRYAGAIN;
	} else {
		return _nss_openvpn_gethostbyname_r(name, result, buffer, buflen, errnop, h_errnop);
	}
}

