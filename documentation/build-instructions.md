# This instruction guide is written for the SailfishOS SDK and Ubuntu 20.04 LTS, but may be also available for other Linux distributions, with some changes.

## SailfishOS SDK

The application can be built and deployed like any normal Sailfish application using the SDK, once some additional dependencies have been installed in the
build target which are not available by default.

To get into the build machine (if started by Sailifsh IDE)
`ssh -l mersdk localhost -p 2222 -i ~/SailfishOS/vmshare/ssh/private_keys/engine/mersdk`

Use the `sb2-config` command to make your ARM target the default

Add this repo which provides KF5 packages
`sb2 -R zypper ar http://repo.merproject.org/obs/home:/piggz:/kf5/sailfishos_3.4.0.24/ piggz`
and
`sb2 -R zypper refresh`

In the .pro file, you can set `FLAVOR = silica` (or `FLAVOR = $$FLAVOR` and in the Sailfish IDE select Project and for Build Settings, add the env variable FLAVOR=silica)

To install missing packages, as pointed out by build errors, use:

```
sb2 -R zypper in kcoreaddons-devel
sb2 -R zypper in kdb-devel
sb2 -R zypper in libKDb3-3
sb2 -R zypper in mkcal-qt5-devel
```

etc as pointed out by build errors.
(although ctrl-B should offer to install them)

## In Ubuntu 20.04, with default Ubuntu repository, QT is in version 5.12.8

### Prepare git repo
Clone amazfish repository:

```
git clone https://github.com/piggz/harbour-amazfish.git`
```

Initialise and sync submodules:
```
git submodule init 
git submodule update
```

### Install dependencies

Two depedencies can be found on git repos
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
git clone https://git.sailfishos.org/mer-core/nemo-qml-plugin-dbus.git
cd nemo-qml-plugin-dbus
qmake
make
sudo make install
```
Other dependencies can be installed from the Ubuntu package repositories:
```
sudo apt install qt5-default libkdb3-dev libkf5contacts-dev libkf5coreaddons-dev qtlocation5-dev qtconnectivity5-dev qtpositioning5-dev qml-module-qtbluetooth
```

### Build process

```
cd harbour-amazfish
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
Once you have built the project, run `sudo make install` to install the service daemon and program files. 

Amazfish will be installed to: `/usr/local/share/harbour-amazfish-ui`

The service will be installed to : `/usr/local/lib/systemd/user/`

Lastly, the service needs to be enabled and then started, as follows:

```
systemctl --user daemon-reload
systemctl --user enable harbour-amazfish
systemctl --user start harbour-amazfish
```
