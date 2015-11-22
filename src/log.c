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
#include <syslog.h>
#include <stdarg.h>

#include "include/log.h"

extern int daemonize;

void log_start(void)
{
	openlog("mountd", LOG_PID, LOG_DAEMON);
}

void log_stop(void)
{
	closelog();
}

void log_printf(char *fmt, ...)
{
	char p[256];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(p, 256, fmt, ap);
	va_end(ap);

	if(daemonize)
		syslog(10, p);
	else
		printf(p);
}
