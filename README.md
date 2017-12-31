Initsystem
==========

An init system (what a surprise!)

Installation
============

Build all the packages in the _initsystem_ and _pkgbuild/core_ directories.


Install the _busybox_ and _initsystem_ packages.  
Configure your hostname, keymap, font and services in /etc/rc.conf.  
Configure your bootloader to use init=/sbin/init-og.


Reboot into your new initsystem.


Install _eudev_, _opentmpfiles_ and _systemd-minimal_ packages. This will replace systemd and libsystemd.  
Install all the packages from the _pkgbuild/core_ directory.
Build and install all the packages in the _pkgbuild/extra_ directory (follow the dependencies).


Additionally, you should add 'initsystem' in the IgnoreGroup entry of /etc/pacman.conf to avoid unneeded upgrade of the packages.
All packages from the _pkgbuild_ directory should be added to the IgnorePkg entry for the same reason.

Work in progress:

- review/rework/clean the configuration directory
- review pkgbuild/extra directory
