This instruction guide is available for ubuntu 20.04 LTS
It may be also available for other Linux distribution, with some changes.

In ubuntu 20.04, with default ubuntu repository, QT is in version 5.12.8


# prepare git repo
Clone amazfish repository
`git clone https://github.com/piggz/harbour-amazfish.git`

sync submodule
`git submodule init && git submodule update`

# manages dependencies

two depedencies can be found on git repos
https://git.sailfishos.org/mer-core/nemo-qml-plugin-dbus
and
https://git.sailfishos.org/mer-core/qtmpris

```
git clone git clone https://git.sailfishos.org/mer-core/qtmpris.git
cd qtmpris
qmake
make
sudo make install
```

```
git clone https://git.sailfishos.org/mer-core/nemo-qml-plugin-dbus.git
qmake
make
sudo make install
```
Others deps should be installed from ubuntu packages
`sudo apt install libkdb3-dev libkf5contacts-dev libkf5coreaddons-dev qtlocation5-dev qtconnectivity5-dev qtpositioning5-dev qml-module-qtbluetooth
`

# build process

```
cd harbour-amazfish
mkdir build
cd build
qmake FLAVOR=kirigami  ..
make
```

Here, you may get an error like this.  
`Project ERROR: Unknown module(s) in QT: CoreAddons`

If you got it, you should edit  qt project file of kcontact `/usr/lib/x86_64-linux-gnu/qt5/mkspecs/modules/qt_KContacts.pri`
and change 
`QT.KContacts.depends = CoreAddons`
with 
`QT.KContacts.depends = KCoreAddons`

This can be done with the following command:
`sudo sed -i 's/QT.KContacts.depends = CoreAddons/QT.KContacts.depends = KCoreAddons/' /usr/lib/x86_64-linux-gnu/qt5/mkspecs/modules/qt_KContacts.pri`

You can now enter again `make` command, and build process should end successfully.
Then `sudo make install`

Amazfish is now located here `/usr/local/share/harbour-amazfish-ui`

Service is located in `/usr/local/lib/systemd/user/`
