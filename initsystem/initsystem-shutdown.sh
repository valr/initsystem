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

PATH="/opt/busybox"
export PATH

cd /
umask 022

################################################################################
# sanity check #################################################################
################################################################################

if [ $PPID != 1 ]; then
  echo "error: this script should have ppid 1"
  exit 1
fi

################################################################################
# terminal settings and sync ###################################################
################################################################################

stty onlcr

echo " "
echo "shutdown: syncing"

sync

################################################################################
# services #####################################################################
################################################################################

#echo "shutdown: stopping services"
#
#sv -w 120 force-stop /etc/init.d/*.sv
#sv exit /etc/init.d/*.sv

################################################################################
# scripts ######################################################################
################################################################################

#echo "shutdown: stopping scripts"
#
#for SCRIPT in `find /etc/init.d -name *.sh -print | sort -r`; do
#  [ -x "${SCRIPT}" ] && "${SCRIPT}" stop
#done

################################################################################
# kill processes ###############################################################
################################################################################

wait_pids() {
  for i in $(seq $1); do
    killall5 -18; # SIGCONT

    if [ $? -eq 2 ]; then
      return 0
    fi

    sleep .25
  done

  return 1
}

echo "shutdown: syncing"

sync

echo "shutdown: sending SIGTERM to processes"

killall5 -15 # SIGTERM
wait_pids 360

if [ $? -ne 0 ]; then

  echo "shutdown: sending SIGKILL to processes"

  killall5 -9 # SIGKILL
  wait_pids 360
fi

################################################################################
# random seed ##################################################################
################################################################################

echo "shutdown: saving random seed"

RANDOM_SEED=/var/lib/misc/random-seed

touch "${RANDOM_SEED}"
chmod 600 "${RANDOM_SEED}"

POOL_FILE=/proc/sys/kernel/random/poolsize

if [ -r "${POOL_FILE}" ]; then
  POOL_SIZE=`cat "${POOL_FILE}"`
else
  POOL_SIZE=512
fi

dd if=/dev/urandom of="${RANDOM_SEED}" count=1 bs=$POOL_SIZE &>/dev/null

################################################################################
# filesystems and swap #########################################################
################################################################################

echo "shutdown: writing wtmp record"

/usr/bin/busybox halt -w

echo "shutdown: unmounting swap-backed filesystems"

for MOUNT in `grep ' tmpfs ' /proc/self/mountinfo | sort -r -k1,1 | cut -d ' ' -f 5`; do
  case "${MOUNT}" in
    /proc|/sys|/run|/dev|/dev/pts|/dev/shm|/sys/firmware/efi/efivars)
      ;;
    *)
      umount -r "${MOUNT}"
      ;;
  esac
done

echo "shutdown: desactivating swap"

swapoff -a

echo "shutdown: unmounting other filesystems"

umount -a -r

echo "shutdown: remounting root filesystem read-only"

mount -o remount,ro /

################################################################################
# finish #######################################################################
################################################################################

case `basename $0` in
  initsystem-halt.sh)
    echo "shutdown: system is halting"
    /usr/bin/busybox halt -f
    ;;
  initsystem-poweroff.sh)
    echo "shutdown: system is shutting down"
    /usr/bin/busybox poweroff -f
    ;;
  initsystem-reboot.sh)
    echo "shutdown: system is rebooting"
    /usr/bin/busybox reboot -f
    ;;
  *)
    echo "error: unexpected shutdown condition, halting now"
    /usr/bin/busybox halt -f
    ;;
esac