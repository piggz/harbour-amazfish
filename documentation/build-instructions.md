# Build Instructions

This instruction guide is written for the SailfishOS SDK and Debian based distributions including Ubuntu Touch, but may be also available for other Linux distributions, with some changes.

## SailfishOS SDK

The application can be built and deployed like any normal Sailfish application using the SDK, once some additional dependencies have been installed in the
build target which are not available by default.

To get into the build machine (if started by Sailfish IDE)
`ssh -l mersdk localhost -p 2222 -i ~/SailfishOS/vmshare/ssh/private_keys/engine/mersdk`

Use the `sb2-config` command to make your ARM target the default

Add this repo which provides KF5 packages
`sb2 -R zypper ar https://repo.sailfishos.org/obs/sailfishos:/chum/4.6_aarch64/ sailfishos_chum`
and
`sb2 -R zypper refresh`

In the .pro file, you can set `FLAVOR = silica` (or `FLAVOR = $$FLAVOR` and in the Sailfish IDE select Project and for Build Settings, add the env variable FLAVOR=silica)

To install missing packages, as pointed out by build errors, use:

```
sb2 -R zypper in mpris-qt5-devel libkf5archive-devel kcoreaddons-devel kdb-devel libKDb3-3 mkcal-qt5-devel libicu-devel pulseaudio-devel
```

etc as pointed out by build errors.
(although ctrl-B should offer to install them)

## Debian based distributions

In Ubuntu 20.04, with default Ubuntu repository, QT is in version 5.12.8. Tested with Mobian (Debian bookworm/sid) and Phosh on the PinePhone Pro.

### Install dependencies

Most dependencies can be installed from the system's package repositories:
```
sudo apt update
sudo apt install -y qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools libkdb3-dev libkf5contacts-dev libkf5archive-dev libkf5coreaddons-dev qtlocation5-dev qtconnectivity5-dev qtpositioning5-dev qml-module-qtbluetooth qml-module-qtquick-controls qml-module-qtquick-controls2 qml-module-qt-labs-settings qml-module-org-kde-kirigami2 qml-module-org-kde-bluezqt
```

Two dependencies can be found on git repos
https://git.sailfishos.org/mer-core/nemo-qml-plugin-dbus
and
https://git.sailfishos.org/mer-core/qtmpris

```
git clone https://git.sailfishos.org/mer-core/qtmpris.git
cd qtmpris
qmake
make
sudo make install
```

```
cd ..
```

```
git clone https://git.sailfishos.org/mer-core/nemo-qml-plugin-dbus.git
cd nemo-qml-plugin-dbus
qmake
make
sudo make install
```

```
cd ..
```

### Prepare git repo

Clone Amazfish repository:
```
git clone https://github.com/piggz/harbour-amazfish.git
cd harbour-amazfish
```

Initialize and sync submodules:

```
git submodule init
git submodule update
```

### Build process

```
mkdir build
cd build
qmake FLAVOR=kirigami  ..
make
```

Here, you may get an error like this:
```
Project ERROR: Unknown module(s) in QT: CoreAddons
```

If you got this error, you should edit the QT project file of KContacts `/usr/lib/x86_64-linux-gnu/qt5/mkspecs/modules/qt_KContacts.pri`
and change 
```
QT.KContacts.depends = CoreAddons`
```
to 
```
QT.KContacts.depends = KCoreAddons
```

This can be done with the following command or an text editor of your choice:
`sudo sed -i 's/QT.KContacts.depends = CoreAddons/QT.KContacts.depends = KCoreAddons/' /usr/lib/x86_64-linux-gnu/qt5/mkspecs/modules/qt_KContacts.pri`

You can now run the `make` command again, and the build process should end successfully.

### Installing Amazfish and enabling background service.
Once you have built the project, run this command to install the service daemon and program files:

```
sudo make install
```

Amazfish will be installed to: `/usr/local/share/harbour-amazfish-ui`

The service will be installed to : `/usr/local/lib/systemd/user/`

Lastly, the service needs to be enabled and then started, as follows:

```
systemctl --user daemon-reload
systemctl --user enable harbour-amazfish
systemctl --user start harbour-amazfish
```

## Ubuntu Touch

Clickable is used to build this app for Ubuntu Touch, see [install instructions](https://clickable-ut.dev/en/latest/install.html).
To build and run the application in Desktop Mode for testing the UI, use the following commands:

```
clickable build --libs
clickable desktop
```

To build the application for ARM architecture and install it on your device, follow these steps:

```
clickable build --libs --app --arch detect --skip-review
clickable install
```

If you do not have a device connected, you can specify the target architecture as `--arch arm64` or `--arch armhf` instead.
