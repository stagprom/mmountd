# Minimalistic (auto) mount daemon
# Copyright (C) 2015  stagprom
#
# This file is part of mmountd.
#
# mmountd is free software: you can redistribute it and/or modify
# i t under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# mmountd is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with mmountd.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.6)

PROJECT(mmountd C)
ADD_DEFINITIONS(-Os -g -ggdb -Wall --std=gnu99 -Wmissing-declarations)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

ADD_EXECUTABLE(mmountd main.c log.c sys.c autofs.c signal.c )
TARGET_LINK_LIBRARIES(mmountd pthread)

INSTALL(TARGETS mmountd
	RUNTIME DESTINATION sbin
)
