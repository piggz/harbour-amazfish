[![Translation status](https://hosted.weblate.org/widget/harbour-amazfish/svg-badge.svg)](https://hosted.weblate.org/engage/harbour-amazfish/)

# Amazfish

Companion application for Huami Devices (such as Amazfit Bip, Cor, MiBand2/3 and GTS and GTS) and the Pinetime Infinitime.

Requires Sailfish version 3.0.3.8 or above due to changes in the base Sailfish image.  Instructions for [compilation](https://github.com/piggz/harbour-amazfish/blob/master/documentation/build-instructions.md) including for other operating systems can be found in the `documentation/` directory.

## Installation
* Install the ["chum" repository](https://chumrpm.netlify.app/) (by downloading the RPM file and running `devel-su pkcon install-local RPM_FILENAME` )
* Run `devel-su pkcon refresh`
* Run `devel-su pkcon install harbour-amazfish`

<a href='https://open-store.io/app/uk.co.piggz.amazfish'><img width='240' alt='Download on OpenStore' src='https://open-store.io/badges/en_US.svg'/></a>
<a href='https://flathub.org/apps/details/uk.co.piggz.amazfish'><img width='240' alt='Download on Flathub' src='https://flathub.org/assets/badges/flathub-badge-en.png'/></a>


## Supported Devices

There are 3 tiers of supported devices:

### Gold

These are devices I have, have tested, and will try not to break any functionality for any included:
 * Amazfit Bip
 * Amazfit GTS
 * Amazfit GTR2
 * Pinetime InfiniTime
 * Bangle.js

### Silver

These are devices which are properly implemented in the application, but I do not have and are tested by the community, and include:
 * Amazfit BipS
 * Amazfit Bip Lite
 * Amazfit GTS2
 
I do not promise not to break functionality in these, but will fix where I can.

### Bronze

These are devices which use a protocol that is close to another supported device, and so is treated like that device.  Your mileage may vary with these devices.
 * MiBand 2/3/4
 * Amazfit Cor
 
## Note for GTR2 / GTS2 owners

These watches fail to follow the Bluetooth specification, which states that the "Appearance" characteristic should be readable without auth/encryption.  In fact, it seems this
characteristic is not readable at all, even though it is available, and this causes a failure in Bluez.  It seems like Android/iOS are less strict in this regard.

A patch has been added to Bluez which makes it not read the characteristic if it has previously been read and cached, and this allows us to work around the issue.

The device cache is in /var/lib/bluetooth/[host mac address]/[watch mac address/info.

As root, add the line

    Appearance=0x0192

under the [General] heading and restart Bluetooth with

    systemctl restart bluetooth

Do this after pairing the watch in Amazfish.

## Powered by KEXI

As of version 0.5.1, activity data is retrieved into an SQLite database.  Because I think it is important to allow individuals to be in control of their own data, and that they should have the ability to analyze it themselves, I have chosen to store data in a KEXI compatible database.  This will allow you to copy the database from the phone, and open it up inside KEXI on Linux/Windows/Mac and perform queries and reports on it.  This added ability means I link to a couple of KDE libraries, which should be installed automatically.

## Firmware Download - READ FIRST - I am not liable if you brick your watch ;)

Read this for info about which files to flash: <https://codeberg.org/Freeyourgadget/Gadgetbridge/wiki/Amazfit-Bip>

File download has been tested using an Amazfit Bip and GTS.

The firmware is split into multiple files on these devices.  With the official MiFit app, all are downloaded automatically in the correct order.  With this app, you have to send each file individually.  The firmware files are available by extracting the Zepp apk, and looking in the assets/ folder for files named Mili_chaohu.* for the Bip.  GTS firmware files can be found online, the device name is kestrelw, but be sure the files you are downloading are legitimate.  When you select a file, its type and version will be determined, and you will be prevented from sending invalid files.

The firmware (.fw) requires a matching font (.ft) and resource (.res).  Send the firmware first, the app will send a reboot command at the end of the transfer, and the watch will boot up into a mode where it needs the matching font and resource sent.  Just wait for the app to connect again, then send the font and resource. 

The following types of file exist:

firmware, resource, a-gps data, fonts

So, to re-iterate, the firmware flashing order is:

    .fw
    .ft
    .res

# Server Side Pairing Code

The newer devices (GTS, BipS etc) requires a server-side pairing key which requires initial pairing with an android device.  To get the key, follow the instructions here and enter into Amazfish when prompted, WITHOUT 0x at the beginning.

See https://codeberg.org/Freeyourgadget/Gadgetbridge/wiki/Amazfit-GTS

*Pairing Issues*

Pairing works for myself, sometimes I have to try twice.  If pairing doesn't work at all, then try:

    Keep the app open, and pair again in the BT settings page of sailfish
    In the sailfish BT settings page, delete all entries and try again
    Turn off/on BT on the phone
    Factory reset the watch

Check if after all this, pairing works from the sailfish BT settings page.  It should pair and connect just fine from there.  If that doesn't work, my app wont either.

Not all features currently implemented.

***May not work on all devices***

Uses Bluetooth Low Energy to communicate with the watch, this is known to be problematic on some devices.  It is known to work on the Xiaomi Mido.  Probably also works on the FP2 and is tested on the XA2.

Implemented
    Pairing
    Notifications
    Calls
    Some settings
    Retrieving activities
    Heartrate Chart
    Alarms
    Watchface download
    Firmware upload
    Activity Sync
    Basic music control

Todo
    More Settings
    Support other devices (maybe e.g. MiBand2, as it is similar)


Tip:
On your device, create a symlink in /home/nemo to /home/nemo/.local/share/harbour-amazfish
This way, you can easily copy the database off the phone using MTP.

## Button Actions

Bip and GTS notify when the watch button is pressed.  This is captured by Amazfish which allows running actions on theses presses.  An action can be ran on 2,3 or 4 presses.
To configure the actions, go to Settings > Application > Button Actions.

A custom script can be used, the script must be called harbour-amazfish-script.sh and be in the home folder.
Example which sends a OTP code to the watch as a message:

    #!/bin/sh
    if [ "$1" == "4" ]; then
        CODE=`python3 -c 'import pyotp;totp = pyotp.TOTP("XXXX Authenticator code here XXXX");print(totp.now())'`
        dbus-send --session --print-reply --dest=uk.co.piggz.amazfish /application uk.co.piggz.amazfish.sendAlert string:'' string:'Google OTP' string:$CODE boolean:true
    fi;

Source: https://github.com/piggz/harbour-amazfish

Credits to:

    The Gadgetbridge devs, which gave me a lot of hints and inspiration from their device code.
    https://codeberg.org/Freeyourgadget/Gadgetbridge
