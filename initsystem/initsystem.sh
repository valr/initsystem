#!/usr/bin/ash

# This file is part of initsystem
#
# Copyright (C) 2014 Valère Monseur (valere dot monseur at ymail dot com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

set -u

PATH="/sbin:/bin:/usr/sbin:/usr/bin"
export PATH

cd /
umask 0

################################################################################
# sanity check #################################################################
################################################################################

if [ $$ != 1 ]; then
  echo "error: this script should be pid 1"
  exit 1
fi

################################################################################
# pid 1 ########################################################################
################################################################################

exec /usr/bin/initsystem \
  "/opt/busybox/syslogd -n -C32768" \
  "/opt/busybox/getty -l /opt/busybox/login 38400 tty1 linux" \
  "/opt/busybox/getty -l /opt/busybox/login 38400 tty2 linux" \
  "/opt/busybox/getty -l /opt/busybox/login 38400 tty3 linux" \
  "/opt/busybox/getty -l /opt/busybox/login 38400 tty4 linux" \
  "/opt/busybox/getty -l /opt/busybox/login 38400 tty5 linux" \
  "/opt/busybox/getty -l /opt/busybox/login 38400 tty6 linux"
