Initsystem
==========

An init system (what a surprise!)

Installation
============

Build and install the main packages from the initsystem directory.

- busybox
- initramfs
- initsystem

Optionally, build and install the following packages from the initsystem 
directory to replace udev and tmpfiles from systemd by eudev and opentmpfiles.
Those packages will require the installation of the systemd stripped package.

- eudev
- opentmpfiles
- systemd

Build and install the packages from the pkgbuild directory to remove 
the dependencies on systemd libraries.

Todo:

- review pkgbuild in extra directory
- cronjobs
- services
