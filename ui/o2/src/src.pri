QT *= network

lessThan(QT_MAJOR_VERSION, 5): QT *= script

INCLUDEPATH += $$PWD
SOURCES += \
    $$PWD/o2.cpp \
    $$PWD/o2reply.cpp \
    $$PWD/o2replyserver.cpp \
    $$PWD/o2requestor.cpp \
    $$PWD/o2simplecrypt.cpp \
    $$PWD/o0baseauth.cpp \
    $$PWD/o0settingsstore.cpp

HEADERS += \
    $$PWD/o2.h \
    $$PWD/o2reply.h \
    $$PWD/o2replyserver.h \
    $$PWD/o2requestor.h \
    $$PWD/o0baseauth.h \
    $$PWD/o0globals.h \
    $$PWD/o0simplecrypt.h \
    $$PWD/o0requestparameter.h \
    $$PWD/o0abstractstore.h \
    $$PWD/o0settingsstore.h
