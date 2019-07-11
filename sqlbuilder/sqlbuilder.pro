DESTDIR = $$PWD/../bin

QT       += core sql

TARGET = sqlbuilder
TEMPLATE = lib
CONFIG += staticlib c++11

SOURCES += \
    Config.cpp \
    Query.cpp \
    Selector.cpp \
    Where.cpp \
    Inserter.cpp \
    Deleter.cpp \
    Updater.cpp

HEADERS += \
    Config.h \
    Query.h \
    Selector.h \
    Where.h \
    Inserter.h \
    Deleter.h \
    Updater.h

DEFINES *= QT_USE_QSTRINGBUILDER
