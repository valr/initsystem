Initsystem
==========

An init system (what a surprise!)

Installation
============

Build all the packages in the _initsystem_ and _pkgbuild_ directories.


Install _busybox_, _initramfs_, _initsystem_ and _services_ packages.
Configure your hostname, keymap, font and services in /etc/rc.conf.
Configure your bootloader to use init=/sbin/init-og.


Reboot into your new initsystem.


Install _eudev_, _opentmpfiles_ and _systemd-minimal_ packages. This will replace systemd and libsystemd.
Install all the packages from the _pkgbuild/core_ directory.


Additionally, you should add 'initsystem' in the IgnoreGroup entry of /etc/pacman.conf to avoid unneeded upgrade from the official repositories.
All packages from the _pkgbuild_ directory should be added to the IgnorePkg entry for the same reason.

Work in progress:

- review/rework/clean the configuration directory
- review pkgbuild/extra directory
