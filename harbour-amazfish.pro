#
# tohkbd2 top level project file
#
TEMPLATE = subdirs
CONFIG = ordered
SUBDIRS = daemon ui
OTHER_FILES =

DISTFILES += \
    rpm/harbour-amazfish.changes.in \
    rpm/harbour-amazfish.changes.run.in \
    rpm/harbour-amazfish.spec \
    rpm/harbour-amazfish.yaml \