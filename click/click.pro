# This creates the manifest.json file, it is the description file for the
# click package.

TEMPLATE = aux

# figure out the current build architecture
CLICK_ARCH=$$system(dpkg-architecture -qDEB_HOST_ARCH)

# do not remove this line, it is required by the IDE even if you do
# not substitute variables in the manifest file
UBUNTU_MANIFEST_FILE = $$PWD/manifest.json.in


# substitute the architecture in the manifest file
manifest_file.output   = manifest.json
manifest_file.CONFIG  += no_link \
                         add_inputs_as_makefile_deps\
                         target_predeps
manifest_file.commands = sed s/@CLICK_ARCH@/$$CLICK_ARCH/g ${QMAKE_FILE_NAME} > ${QMAKE_FILE_OUT}
manifest_file.input = UBUNTU_MANIFEST_FILE
QMAKE_EXTRA_COMPILERS += manifest_file

# installation path of the manifest file
mfile.CONFIG += no_check_exist
mfile.files  += $$OUT_PWD/manifest.json
mfile.path = /

# AppArmor profile
apparmor.files += $$PWD/harbour-amazfish.apparmor
apparmor.path = /

# Desktop launcher icon
desktopicon.files += $$PWD/harbour-amazfish-ui.png
desktopicon.path = /

# Desktop launcher
desktop.files += $$PWD/harbour-amazfish-ui.desktop
desktop.path = /

# Run script
runscript.files += $$PWD/run.sh
runscript.path = /

INSTALLS += mfile apparmor runscript desktopicon desktop
