
Debian
====================
This directory contains files used to package bitwin24d/bitwin24-qt
for Debian-based Linux systems. If you compile bitwin24d/bitwin24-qt yourself, there are some useful files here.

## bitwin24: URI support ##


bitwin24-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install bitwin24-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your bitwin24qt binary to `/usr/bin`
and the `../../share/pixmaps/bitwin24128.png` to `/usr/share/pixmaps`

bitwin24-qt.protocol (KDE)

