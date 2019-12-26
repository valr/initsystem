Initsystem
==========

An init system (what a surprise!) for [Arch Linux](https://www.archlinux.org)

Installation
============

Build all the packages in the _initsystem_ and _pkgbuild/core_ directories.


Install the _busybox_ and _initsystem_ packages.  
Configure your hostname, keymap, font and services in /etc/rc.conf.  
Configure your bootloader to use init=/sbin/init-og.


Reboot into your new initsystem.


Install _eudev_, _opentmpfiles_ and _systemd_ (minimal) packages. This will replace the official systemd and systemd-libs packages.  
Install all the packages in the _pkgbuild/core_ directory.  


If needed, build and install the packages in the _pkgbuild/extra_ directory (carefully follow the build dependencies!).


Additionally, you should add 'initsystem' to the IgnoreGroup entry of /etc/pacman.conf to avoid unneeded upgrade of the initsystem packages.
All packages in the _pkgbuild_ directories should be added to the IgnorePkg entry for the same reason.


Finally, review and adapt your configuration to remove the systemd specific parameters, files, ... (there are just too many to list them all).
