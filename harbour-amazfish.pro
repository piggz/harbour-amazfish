TEMPLATE = subdirs
CONFIG = ordered

exists("/usr/lib/qt5/qml/Sailfish/Silica/SilicaGridView.qml"): {
    message(SailfishOS build)
    DEFINES += MER_EDITION_SAILFISH
    SUBDIRS = lib daemon ui
} else {
    SUBDIRS = lib daemon
}
DISTFILES += \
    rpm/harbour-amazfish.changes.in \
    rpm/harbour-amazfish.changes.run.in \
    rpm/harbour-amazfish.spec \
    rpm/harbour-amazfish.yaml \
