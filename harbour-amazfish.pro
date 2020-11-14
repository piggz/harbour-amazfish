TEMPLATE = subdirs
CONFIG = ordered

# Find out flavor and add it to CONFIG for simple testing
equals(FLAVOR, "silica") {
    CONFIG += flavor_silica
} else:equals(FLAVOR, "kirigami") {
    CONFIG += flavor_kirigami
} else:equals(FLAVOR, "qtcontrols") {
    CONFIG += flavor_qtcontrols
} else:equals(FLAVOR, "ubports") {
    CONFIG += flavor_ubports
} else {
    error("Please specify platform using FLAVOR=platform as qmake option. Supported platforms: kirigami, silica, qtcontrols, ubports.")
}

flavor_silica {
    message(SailfishOS build)
    CONFIG += sailfishapp sailfishapp_no_deploy_qml
    DEFINES += MER_EDITION_SAILFISH
}

# PREFIX
isEmpty(PREFIX) {
    flavor_silica {
        PREFIX = /usr
    } else:flavor_ubports {
        PREFIX = /
    } else {
        PREFIX = /usr/local
    }
}

# PREFIX_RUNNING
isEmpty(PREFIX_RUNNING) {
    flavor_ubports {
        PREFIX_RUNNING = .
    } else {
        PREFIX_RUNNING = $$PREFIX
    }
}


DATADIR = $$PREFIX/share/$${TARGET}
#End install config

SUBDIRS = lib daemon ui

DISTFILES += \
    rpm/harbour-amazfish.changes.in \
    rpm/harbour-amazfish.changes.run.in \
    rpm/harbour-amazfish.spec \
    rpm/harbour-amazfish.yaml \
