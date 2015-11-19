/******************************************************************************
Minimalistic (auto) mount daemon
Copyright (C) 2015  stagprom

This file is part of mmountd.

mmountd is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

mmountd is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with mmountd.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************************/

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "include/log.h"
#include "include/signal.h"


void signal_init(void (*sig_handler)(int s)) {
	struct sigaction sig;
	
	sig.sa_handler = sig_handler;
	sig.sa_flags = 0;
	sigaction(SIGTERM, &sig, NULL);
	sigaction(SIGINT, &sig, NULL);
	sigaction(SIGHUP, &sig, NULL);
	sigaction(SIGQUIT, &sig, NULL);
}
