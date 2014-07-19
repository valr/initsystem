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
# in-memory filesystems ########################################################
################################################################################

echo "initscript: mounting in-memory filesystems"

mount -t proc -o nosuid,nodev,noexec proc /proc
mount -t sysfs -o nosuid,nodev,noexec sys /sys
mount -t tmpfs -o nosuid,nodev,mode=0755 run /run
mount -t devtmpfs -o nosuid,mode=0755 dev /dev
mkdir -p /dev/pts
mount -t devpts -o nosuid,noexec,gid=5,mode=0620,ptmxmode=0 pts /dev/pts
mkdir -p /dev/shm
mount -t tmpfs -o nosuid,nodev shm /dev/shm
mount -t efivarfs -o nosuid,nodev,noexec efi /sys/firmware/efi/efivars

################################################################################
# filesystems ##################################################################
################################################################################

echo "initscript: remounting root filesystem read-write"

mount -o remount,rw /

################################################################################
# hostname #####################################################################
################################################################################

if [ -s /etc/hostname ]; then
  echo "initscript: setting hostname"

  hostname -F /etc/hostname
fi

################################################################################
# hardware clock ###############################################################
################################################################################

echo "initscript: setting clock"

hwclock -t -u

################################################################################
# devices ######################################################################
################################################################################

echo "initscript: starting devices handler"

/usr/lib/systemd/systemd-udevd --daemon

/usr/bin/udevadm trigger --action=add --type=subsystems
/usr/bin/udevadm trigger --action=add --type=devices

################################################################################
# modules ######################################################################
################################################################################

echo "initscript: loading modules"

DIR1="/usr/lib/modules-load.d"
DIR2="/run/modules-load.d"
DIR3="/etc/modules-load.d"

for FILE in `find "${DIR1}" "${DIR2}" "${DIR3}" -name *.conf -exec basename '{}' \; 2> /dev/null | sort -u`; do

  [ -r "${DIR1}/${FILE}" ] && IFILE="${DIR1}/${FILE}"
  [ -r "${DIR2}/${FILE}" ] && IFILE="${DIR2}/${FILE}"
  [ -r "${DIR3}/${FILE}" ] && IFILE="${DIR3}/${FILE}"

  grep -E -v "^[[:blank:]]*(#|;|$)" "${IFILE}" | while read MOD; do
    modprobe "${MOD}"
  done
done

################################################################################
# devices ######################################################################
################################################################################

echo "initscript: waiting devices to settle"

/usr/bin/udevadm settle

################################################################################
# console ######################################################################
################################################################################

if [ -s /etc/vconsole.conf ]; then
  echo "initscript: setting console keymap and font"

  KEYMAP=`grep KEYMAP /etc/vconsole.conf | cut -d '=' -f 2`

  if [ ! -z "${KEYMAP}" ]; then
    /usr/bin/loadkeys "${KEYMAP}"
  fi

  FONT=`grep FONT /etc/vconsole.conf | cut -d '=' -f 2`

  if [ ! -z "${FONT}" ]; then
    for TTY in /dev/tty[0-9]*; do
      /usr/bin/setfont "$FONT" -C "${TTY}" &> /dev/null
    done
  fi
fi

################################################################################
# loopback #####################################################################
################################################################################

if [ -d /sys/class/net/lo ]; then
  echo "initscript: setting loopback device"

  ip link set up dev lo
fi

################################################################################
# filesystems ##################################################################
################################################################################

echo "initscript: mounting other filesystems"

mount -a

################################################################################
# swap #########################################################################
################################################################################

echo "initscript: activating swap"

swapon -a

################################################################################
# random number generator ######################################################
################################################################################

echo "initscript: initializing random number generator"

RANDOM_SEED=/var/lib/misc/random-seed

if [ -f "${RANDOM_SEED}" ]; then
  cat "${RANDOM_SEED}" > /dev/urandom
else
  touch "${RANDOM_SEED}"
fi

chmod 600 "${RANDOM_SEED}"

POOL_FILE=/proc/sys/kernel/random/poolsize

if [ -r "${POOL_FILE}" ]; then
  POOL_SIZE=`cat "${POOL_FILE}"`
else
  POOL_SIZE=512
fi

dd if=/dev/urandom of="${RANDOM_SEED}" count=1 bs=$POOL_SIZE &>/dev/null

################################################################################
# volatile and temporary files #################################################
################################################################################

echo "initscript: setting volatile and temporary files"

/usr/bin/systemd-tmpfiles --create --remove --exclude-prefix=/dev
rm -f /run/nologin

################################################################################
# sysctl #######################################################################
################################################################################

echo "initscript: setting kernel parameters"

DIR1="/usr/lib/sysctl.d"
DIR2="/run/sysctl.d"
DIR3="/etc/sysctl.d"

for FILE in `find "${DIR1}" "${DIR2}" "${DIR3}" -name *.conf -exec basename '{}' \; 2> /dev/null | sort -u`; do

  [ -r "${DIR1}/${FILE}" ] && IFILE="${DIR1}/${FILE}"
  [ -r "${DIR2}/${FILE}" ] && IFILE="${DIR2}/${FILE}"
  [ -r "${DIR3}/${FILE}" ] && IFILE="${DIR3}/${FILE}"

  sysctl -q -p "${IFILE}"
done

################################################################################
# dmesg ########################################################################
################################################################################

echo "initscript: saving dmesg"

umask 077
dmesg > /var/log/dmesg
umask 022

################################################################################
# scripts ######################################################################
################################################################################

#echo "initscript: starting scripts"
#
#for SCRIPT in `find /etc/init.d -name *.sh -print | sort`; do
#  [ -x "${SCRIPT}" ] && "${SCRIPT}" start
#done

################################################################################
# services #####################################################################
################################################################################

# todo