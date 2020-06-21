# Initsystem

An init system (what a surprise!) for [Arch Linux](https://www.archlinux.org)

# Installation

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

# Documentation

## Initsystem

Initsystem is a very simple init system for Arch Linux.  
It is based on busybox and shell scripts.  

The [init process](https://github.com/valr/initsystem/blob/master/initsystem/initsystem/src/init-og.c) is a small C program responsible to:
* run the startup script [/etc/rc](https://github.com/valr/initsystem/blob/master/initsystem/initsystem/etc/rc) and shutdown script [/etc/rc.shutdown](https://github.com/valr/initsystem/blob/master/initsystem/initsystem/etc/rc.shutdown)
* run the [/etc/respawn](https://github.com/valr/initsystem/blob/master/initsystem/initsystem/etc/rc.respawn) script to (re)spawn getty's and invoke the login commands
* reap the zombie processes

The initsystem and [services](https://github.com/valr/initsystem/tree/master/initsystem/initsystem/etc/rc.d) are configured in [/etc/rc.conf](https://github.com/valr/initsystem/blob/master/initsystem/initsystem/etc/rc.conf).  
Extra commands can be configured in [/etc/rc.local](https://github.com/valr/initsystem/blob/master/initsystem/initsystem/etc/rc.local) and [/etc/rc.shutdown.local](https://github.com/valr/initsystem/blob/master/initsystem/initsystem/etc/rc.shutdown.local).  

The system can be stopped/restarted using one of the commands: halt, poweroff or reboot.  
Alternative commands are [shutdown](https://github.com/valr/initsystem/blob/master/initsystem/initsystem/bin/shutdown) -h, shutdown -p or shutdown -r respectively.  

## Initramfs

The [initramfs](https://github.com/valr/initsystem/blob/master/initsystem/initsystem/libexec/initramfs) is also based on busybox and shell scripts.
It is [generated](https://github.com/valr/initsystem/blob/master/initsystem/initsystem/bin/mkinitramfs) by a pacman [hook](https://github.com/valr/initsystem/blob/master/initsystem/initsystem/archlinux/zz-initramfs.hook) [script](https://github.com/valr/initsystem/blob/master/initsystem/initsystem/archlinux/zz-initramfs).  
It includes the modules and firmwares discovered by the Arch Linux initramfs.  

## Packages

* Statically compiled [busybox](https://www.busybox.net/) with almost all commands included
* The device manager is [eudev](https://wiki.gentoo.org/wiki/Project:Eudev) from Gentoo Linux
* The tmpfiles utility is [opentmpfiles](https://github.com/OpenRC/opentmpfiles) from Gentoo's OpenRC
* Users and groups are managed manually (opensysusers is not used)
* A minimal [systemd](https://github.com/systemd/systemd) package is created to:
    * provide fake but necessary dependencies on systemd, systemd-libs, libsystemd, systemd-sysvcompat
    * provide standard directories (modules-load, sysctl.d, tmpfiles.d, systemd unit directories)
    * provide standard systemd tmpfiles files
    * provide a compare tool run as pacman pre & post hook scripts allowing follow-up of additions, changes, deletions of systemd unit files
