Initsystem
==========

An init system (what a surprise!)

Installation
============

Build all the packages in the _initsystem_ directory.  
Install the _busybox_ and _initsystem_ packages.

Configure your hostname, keymap, font and services in /etc/rc.conf.  
Configure your bootloader to use init=/sbin/init-og.


Reboot into your new initsystem.


Install _eudev_, _opentmpfiles_ and _systemd_ (minimal) packages. This will replace the official systemd and libsystemd packages.


If needed, build and install the packages in the _pkgbuild/extra_ directory (follow the dependencies).


Additionally, you should add 'initsystem' to the IgnoreGroup entry of /etc/pacman.conf to avoid unneeded upgrade of the initsystem packages.
All packages in the _pkgbuild_ directories should be added to the IgnorePkg entry for the same reason.
