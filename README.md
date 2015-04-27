Initsystem
==========

An init system (what a surprise!)

Installation
============

Build and install the following packages:

- busybox
- initramfs
- initsystem
- services
- cronjobs
- eudev (optional: removes systemd and replaces udev components from systemd)
- packages in pkgbuild directory (optional: removes dependencies on systemd libraries)

Configure the following components:

- initramfs: module and firmware files in /etc/mkinitramfs.d
- initsystem: /etc/rc.conf
