Initsystem
==========

An init system (what a surprise!)

Installation
============

Build and install the following packages:

- busybox
- fsck-fat (for fat & vfat filesystems)
- fsck-ext (for ext3 & ext4 filesystems)
- initramfs
- initsystem
- services
- cronjobs
- eudev (optional: removes systemd and replaces udev components from systemd)
- all packages in pkgbuild directory (removes dependencies on systemd libraries)

Configure the following components:

- initramfs: modules file(s) in /etc/mkinitramfs.d
- initsystem: /etc/rc.conf
