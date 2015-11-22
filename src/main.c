/*
 *  Minimalistic (auto) mount daemon
 *  Copyright (C) 2015  Ernest Vogelsang <stagprom@posteo.de>
 * 
 *  This file is part of mmountd.
 * 
 *  mmountd is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  mmountd is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with mmountd.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *  Original provided by fon.com
 *  Copyright (C) 2009 John Crispin <blogic@openwrt.org> 
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "include/log.h"
#include "include/sys.h"
#include "include/autofs.h"

int daemonize = 1;

void usage(void) {
	fprintf(stderr,"usage: mmountd timeout mountopt device mountpoint [helper]\n");
}

int main(int argc, char *argv[])
{
	int err;
	
	if(argc < 4) {
		usage();
		return 1;
	}
	log_start();
	log_printf("Starting OpenWrt minimal(auto)mountd V1\n");
	if (geteuid() != 0) {
		fprintf(stderr, "This program must be run by root.\n");
		return 1;
	}
	timeout = strtol( argv[1], NULL, 10 );
	mountopt = argv[2];
	device = argv[3];
	mountp = argv[4];
	helper = argv[5];
	autofs_init();
	autofs_loop();
	return(0);
}
