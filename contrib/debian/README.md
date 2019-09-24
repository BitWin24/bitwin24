
Debian
====================
This directory contains files used to package magd/bitwin24-qt
for Debian-based Linux systems. If you compile magd/bitwin24-qt yourself, there are some useful files here.

## bitwin24: URI support ##


bitwin24-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install bitwin24-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your magqt binary to `/usr/bin`
and the `../../share/pixmaps/mag128.png` to `/usr/share/pixmaps`

bitwin24-qt.protocol (KDE)

