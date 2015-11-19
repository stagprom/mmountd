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

#ifndef _SIGNAL_H__
#define _SIGNAL_H__

void signal_init(void (*sig_handler)(int s));

int signal_usr1(void);
int signal_usr2(void);

#endif
